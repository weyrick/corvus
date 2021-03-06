/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PSOURCEMANAGER_H_
#define COR_PSOURCEMANAGER_H_

#include "corvus/pTypes.h"
#include "corvus/pSourceModule.h"
#include "corvus/pConfig.h"

#include <ostream>
#include <map>
#include <string>

class sqlite3;

namespace corvus {

class pPassManager;
class pModel;

class pSourceManager {
public:
    typedef std::vector<pSourceModule*> DiagModuleListType;

private:
    typedef std::map<std::string, pSourceModule*> ModuleListType;
    typedef std::map<pSourceModule*, bool> DiagTrackerType;

    bool debugParse_, debugModel_, debugDiags_;
    int verbosity_;    
    ModuleListType moduleList_;

    // the source modules from moduleList_ which have diagnostics waiting
    // note that moduleList_ is the owner of these pointers, not diagModuleList_
    DiagTrackerType diagModuleTracker_;

    sqlite3 *db_;
    pModel *model_;
    std::string dbName_;
    std::ostream *logStream_;

    void log(llvm::Twine msg, int verbosity = 1) {
        if (logStream_ && verbosity_ >= verbosity) {
            *logStream_ << msg.str() << std::endl;
        }
    }

    void runPasses(pPassManager *pm);
    void openModel();

public:

    pSourceManager(std::ostream *logStream = 0): debugParse_(false),
        debugModel_(false),
        debugDiags_(false),
        verbosity_(0),
        db_(NULL),
        model_(NULL),
        logStream_(logStream),
        dbName_() { }
    ~pSourceManager();

    void setLogStream(std::ostream *logStream) { logStream_ = logStream; }
    void setModelDBName(pStringRef db)  { dbName_ = db; }

    void configure(const pConfig& config);

    void printAST();
    void printToks();

    void addSourceFile(pStringRef name);
    void addSourceDir(pStringRef name, pStringRef exts);
    void addIncludeDir(pStringRef name, pStringRef exts);

    void refreshModel(pStringRef graphFileName="");
    void runDiagnostics();

    const pModel* model() { return model_; }

    pSourceModule* getSourceModuleByRealpath(pStringRef name);

    // caller will not own these and should not free them
    void trackDiagModule(pSourceModule *m) {
        diagModuleTracker_[m] = true;
    }

    DiagModuleListType getDiagModules();


};

} // namespace

#endif
