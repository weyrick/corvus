/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/pSourceManager.h"

#include "corvus/pLexer.h"
#include "corvus/pSourceModule.h"
#include "corvus/pPassManager.h"
#include "corvus/pModel.h"
#include "corvus/pFullModelChecker.h"
#include "corvus/pParseError.h"
#include "corvus/pDiagnostic.h"

#include "corvus/passes/PrintAST.h"
#include "corvus/passes/DumpStats.h"
#include "corvus/passes/Trivial.h"
#include "corvus/passes/ModelBuilder.h"
#include "corvus/passes/ModelChecker.h"
#include "corvus/passes/TypeAnalysis.h"

#include <llvm/Support/Path.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/ADT/SmallVector.h>

#include <sqlite3.h>

#include <stdlib.h>
#include <assert.h>
#include <system_error>                 // for error_code

namespace corvus { 

/*
 * http://www.sqlite.org/backup.html
 *
** This function is used to load the contents of a database file on disk
** into the "main" database of open database connection pInMemory, or
** to save the current contents of the database opened by pInMemory into
** a database file on disk. pInMemory is probably an in-memory database,
** but this function will also work fine if it is not.
**
** Parameter zFilename points to a nul-terminated string containing the
** name of the database file on disk to load from or save to. If parameter
** isSave is non-zero, then the contents of the file zFilename are
** overwritten with the contents of the database opened by pInMemory. If
** parameter isSave is zero, then the contents of the database opened by
** pInMemory are replaced by data loaded from the file zFilename.
**
** If the operation is successful, SQLITE_OK is returned. Otherwise, if
** an error occurs, an SQLite error code is returned.
*/
namespace {
int loadOrSaveDb(sqlite3 *pInMemory, const char *zFilename, int isSave){
  int rc;                   /* Function return code */
  sqlite3 *pFile;           /* Database connection opened on zFilename */
  sqlite3_backup *pBackup;  /* Backup object used to copy data */
  sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
  sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

  /* Open the database file identified by zFilename. Exit early if this fails
  ** for any reason. */
  rc = sqlite3_open(zFilename, &pFile);
  if( rc==SQLITE_OK ){

    /* If this is a 'load' operation (isSave==0), then data is copied
    ** from the database file just opened to database pInMemory.
    ** Otherwise, if this is a 'save' operation (isSave==1), then data
    ** is copied from pInMemory to pFile.  Set the variables pFrom and
    ** pTo accordingly. */
    pFrom = (isSave ? pInMemory : pFile);
    pTo   = (isSave ? pFile     : pInMemory);

    /* Set up the backup procedure to copy from the "main" database of
    ** connection pFile to the main database of connection pInMemory.
    ** If something goes wrong, pBackup will be set to NULL and an error
    ** code and  message left in connection pTo.
    **
    ** If the backup object is successfully created, call backup_step()
    ** to copy data from pFile to pInMemory. Then call backup_finish()
    ** to release resources associated with the pBackup object.  If an
    ** error occurred, then  an error code and message will be left in
    ** connection pTo. If no error occurred, then the error code belonging
    ** to pTo is set to SQLITE_OK.
    */
    pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
    if( pBackup ){
      (void)sqlite3_backup_step(pBackup, -1);
      (void)sqlite3_backup_finish(pBackup);
    }
    rc = sqlite3_errcode(pTo);
  }

  /* Close the database connection opened on database file zFilename
  ** and return the result of this function. */
  (void)sqlite3_close(pFile);
  return rc;
}
}


pSourceManager::~pSourceManager() {

    if (db_) {
        if (!dbName_.empty()) {
            log("flushing in memory db to: " + dbName_, 2);
            int rc = loadOrSaveDb(db_, dbName_.c_str(), 1);
            if (rc != SQLITE_OK) {
                std::cerr << "failing saving in memory db!" << std::endl;
            }
        }
        sqlite3_close(db_);
    }

    if (model_)
        delete model_;

    for (ModuleListType::iterator i = moduleList_.begin();
         i != moduleList_.end();
         i++) {
        delete i->second;
    }
}

void pSourceManager::configure(const pConfig& config) {

    std::vector<std::string> inputFiles;

    verbosity_ = config.verbosity;
    debugParse_ = config.debugParse;
    debugModel_ = config.debugModel;
    debugDiags_ = config.debugDiags;

    if (!config.rootDir.empty()) {
        log("[config] switching to rootDir: " + config.rootDir);
        chdir(config.rootDir.c_str());
    }

    // set values from config
    if (!config.dbName.empty()) {
        log("[config] setting db name: " + config.dbName);
        setModelDBName(config.dbName);
    }
    if (!config.includePaths.empty()) {
        for (unsigned i = 0; i != config.includePaths.size(); ++i) {
            log("[config] adding include path: " + config.includePaths[i]);
            addIncludeDir(config.includePaths[i], config.exts);
        }
    }

    if (!config.inputFiles.empty()) {
        for (unsigned i = 0; i != config.inputFiles.size(); ++i) {
            log("[config] adding input file: " + config.inputFiles[i]);
            inputFiles.push_back(config.inputFiles[i]);
        }
    }

    if (inputFiles.empty()) {
        log("[config] no input files");
        return;
    }

    for (unsigned i = 0; i != inputFiles.size(); ++i) {

        llvm::sys::fs::file_status stat;
        llvm::sys::fs::status(inputFiles[i], stat);
        if (llvm::sys::fs::is_directory(stat)) {
            addSourceDir(inputFiles[i], config.exts);
        }
        else if (llvm::sys::fs::is_regular_file(stat)) {
            addSourceFile(inputFiles[i]);
        }
        else {
            log("[config] skipping unknown path: " + inputFiles[i]);
        }

    }

}

void pSourceManager::addSourceFile(pStringRef name) {

    // only add it once, based on realpath
    char *rp = realpath(name.str().c_str(), NULL);
    if (!rp)
        return;

    if (moduleList_.find(rp) != moduleList_.end()) {
        free(rp);
        return;
    }

    log("adding source file: " + name.str());

    pSourceModule* unit(new pSourceModule(this, rp));
    moduleList_[rp] = unit;

    free(rp);
}

pSourceModule* pSourceManager::getSourceModuleByRealpath(pStringRef name) {
    pSourceManager::ModuleListType::iterator i = moduleList_.find(name);
    if (i != moduleList_.end()) {
        return i->second;
    }
    else {
        return NULL;
    }
}

void pSourceManager::addSourceDir(pStringRef name, pStringRef exts) {

    llvm::SmallVector<pStringRef, 8> extList;
    exts.split(extList, ",");

    std::error_code ec;
    for (llvm::sys::fs::recursive_directory_iterator dir(name, ec), dirEnd;
         dir != dirEnd && !ec; dir.increment(ec)) {

        bool found = false;
        for (int i = 0; i < extList.size(); i++) {
            llvm::Twine final(".",extList[i]);
            if (llvm::sys::path::extension(dir->path()) == final.str()) {
                found = true;
                break;
            }
        }

        if (found)
            addSourceFile(dir->path());
    }

}

void pSourceManager::runPasses(pPassManager *pm) {

    for (ModuleListType::iterator i = moduleList_.begin();
         i != moduleList_.end();
         i++) {

        try {
            if (verbosity_ > 1 && i->second->getAST()) {
                log("parsing: " + i->second->fileName());
            }
            // this is idempotent
            i->second->parse(debugParse_);
        }
        catch (pParseError& p) {
            // diag the parse error
            i->second->addDiagnostic(new pDiagnostic(p.loc(), p.what()));
            continue;
        }
        catch (std::exception& e) {
            log("[error]: " + pStringRef(e.what()));
            continue;
        }

        try {
            // run selected passes
            pm->run(i->second, verbosity_);
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

    } // input file loop

}

void pSourceManager::printAST() {

    pPassManager passManager(NULL);

    passManager.addPass<AST::Pass::PrintAST>();
    if (debugParse_)
        passManager.addPass<AST::Pass::DumpStats>();

    runPasses(&passManager);

}

void pSourceManager::printToks() {

    for (ModuleListType::iterator i = moduleList_.begin();
         i != moduleList_.end();
         i++) {

        lexer::pLexer l(i->second->source());
        l.dumpTokens();

    }

}


void pSourceManager::runDiagnostics() {

    pPassManager passManager(model_);

    // standard diag passes
    passManager.addPass<AST::Pass::Trivial>();
    // individual source module model checks
    passManager.addPass<AST::Pass::ModelChecker>();

    runPasses(&passManager);

    // now run full model checks
    pFullModelChecker fmc(this, model_);
    fmc.run();

}

void pSourceManager::addIncludeDir(pStringRef name, pStringRef exts) {

    if (!model_) {
        openModel();
    }

    std::vector<pSourceModule*> includeList;
    llvm::SmallVector<pStringRef, 8> extList;
    exts.split(extList, ",", 8);

    std::error_code ec;
    for (llvm::sys::fs::recursive_directory_iterator dir(name, ec), dirEnd;
         dir != dirEnd && !ec; dir.increment(ec)) {

        bool found = false;
        for (int i = 0; i < extList.size(); i++) {
            llvm::Twine final(".",extList[i]);
            if (llvm::sys::path::extension(dir->path()) == final.str()) {
                found = true;
                break;
            }
        }

        if (found)
            includeList.push_back(new pSourceModule(this, dir->path()));
    }

    pPassManager passManager(model_);
    passManager.addPass<AST::Pass::TypeAnalysis>();
    passManager.addPass<AST::Pass::ModelBuilder>();
    for (std::vector<pSourceModule*>::iterator i = includeList.begin();
         i != includeList.end();
         i++) {

        try {
            // this is idempotent
            log("parsing include file: " + (*i)->fileName());
            (*i)->parse(debugParse_);
        }
        catch (pParseError& p) {
            // diag the parse error
            (*i)->addDiagnostic(new pDiagnostic(p.loc(), p.what()));
            continue;
        }
        catch (std::exception& e) {
            log("[error]: " + pStringRef(e.what()));
            continue;
        }

        try {
            // run selected passes
            passManager.run(*i, verbosity_);
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

    }

    for (std::vector<pSourceModule*>::iterator i = includeList.begin();
         i != includeList.end();
         i++) {
        delete (*i);
    }

}

void pSourceManager::refreshModel(pStringRef graphFileName) {

    if (!model_) {
        openModel();
    }
    model_->setTrace(debugModel_);
    model_->begin();
    pPassManager passManager(model_);
    passManager.addPass<AST::Pass::TypeAnalysis>();
    passManager.addPass<AST::Pass::ModelBuilder>();
    runPasses(&passManager);
    model_->commit();
    model_->resolveClassRelations();
    model_->refreshClassModel(graphFileName);
    model_->setTrace(debugDiags_);

}

void pSourceManager::openModel() {

    assert(!model_);
    assert(!db_);

    // we always use in memory db here, loading and saving at the start/end
    int rc = sqlite3_open("", &db_);
    if (rc) {
        std::cerr << "unable to open in memory model db: " <<
                     sqlite3_errmsg(db_);
        exit(1);
    }

    // try to load an existing db, if we are using one
    if (!dbName_.empty()) {
        log("using model db at: " + dbName_);
        // we ignore a failure here if it doesn't exist yet
        loadOrSaveDb(db_, dbName_.c_str(), 0);
    }

    model_ = new pModel(db_, debugModel_);

}

pSourceManager::DiagModuleListType pSourceManager::getDiagModules() {

    pSourceManager::DiagModuleListType result;
    for (DiagTrackerType::iterator i = diagModuleTracker_.begin();
         i != diagModuleTracker_.end();
         ++i) {
        result.push_back(i->first);
    }
    return result;

}

} // namespace

