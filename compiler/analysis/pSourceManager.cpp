/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/pSourceManager.h"

#include "corvus/analysis/pLexer.h"
#include "corvus/analysis/pSourceModule.h"
#include "corvus/analysis/pPassManager.h"
#include "corvus/analysis/pModel.h"

#include "corvus/analysis/passes/PrintAST.h"
#include "corvus/analysis/passes/DumpStats.h"
#include "corvus/analysis/passes/Trivial.h"
#include "corvus/analysis/passes/ModelBuilder.h"
#include "corvus/analysis/passes/ModelChecker.h"

#include <llvm/Support/PathV2.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/system_error.h>

#include <sqlite3.h>

#include <stdlib.h>
#include <assert.h>

namespace corvus { 

pSourceManager::~pSourceManager() {

    if (db_)
        sqlite3_close(db_);
    if (model_)
        delete model_;

    for (ModuleListType::iterator i = moduleList_.begin();
         i != moduleList_.end();
         i++) {
        delete i->second;
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

    pSourceModule* unit(new pSourceModule(rp));
    moduleList_[rp] = unit;

    free(rp);
}

void pSourceManager::addSourceDir(pStringRef name, pStringRef glob) {

    llvm::error_code ec;
    for (llvm::sys::fs::recursive_directory_iterator dir(name, ec), dirEnd;
         dir != dirEnd && !ec; dir.increment(ec)) {

      if (llvm::sys::path::extension(dir->path()) != glob)
        continue;

      addSourceFile(dir->path());
    }

}

void pSourceManager::runPasses(pPassManager *pm) {

    for (ModuleListType::iterator i = moduleList_.begin();
         i != moduleList_.end();
         i++) {

        try {
            // catch parse errors
            // this is idempotent
            i->second->parse(debugParse_);
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            continue;
        }

        try {
            // run selected passes
            pm->run(i->second);
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
    passManager.addPass<AST::Pass::ModelChecker>();

    runPasses(&passManager);

}

void pSourceManager::readStubs(pStringRef dirName) {

    if (!model_) {
        openModel();
    }

    std::string glob = ".php";

    std::vector<pSourceModule*> stubList;

    llvm::error_code ec;
    for (llvm::sys::fs::recursive_directory_iterator dir(dirName, ec), dirEnd;
         dir != dirEnd && !ec; dir.increment(ec)) {

      if (llvm::sys::path::extension(dir->path()) != glob)
        continue;

      stubList.push_back(new pSourceModule(dir->path()));
    }

    pPassManager passManager(model_);
    passManager.addPass<AST::Pass::ModelBuilder>();
    for (std::vector<pSourceModule*>::iterator i = stubList.begin();
         i != stubList.end();
         i++) {

        try {
            // catch parse errors
            // this is idempotent
            (*i)->parse(debugParse_);
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            continue;
        }

        try {
            // run selected passes
            passManager.run(*i);
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

    }

    for (std::vector<pSourceModule*>::iterator i = stubList.begin();
         i != stubList.end();
         i++) {
        delete (*i);
    }

    model_->commit();

}

void pSourceManager::refreshModel() {

    if (!model_) {
        openModel();
    }

    pPassManager passManager(model_);
    passManager.addPass<AST::Pass::ModelBuilder>();
    runPasses(&passManager);
    model_->commit();

}

void pSourceManager::openModel() {

    assert(!model_);
    assert(!db_);

    //std::string filename = "corvus.db";
    //std::string filename = ":memory:";
    std::string filename = ""; // tmp db, mostly in memory except for spillover

    int rc = sqlite3_open(filename.c_str(), &db_);
    if (rc) {
        std::cerr << "unable to open model db " << filename << ": " <<
                     sqlite3_errmsg(db_);
        exit(1);
    }

    model_ = new pModel(db_, debugModel_);

}

} // namespace

