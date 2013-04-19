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
#include "corvus/analysis/pSourceModule.h"

#include <map>
#include <string>

class sqlite3;

namespace corvus {

class pPassManager;
class pModel;

class pSourceManager {

private:

    bool debugParse_, debugModel_;
    typedef std::map<std::string, pSourceModule*> ModuleListType;
    ModuleListType moduleList_;

    sqlite3 *db_;
    pModel *model_;

    void runPasses(pPassManager *pm);

    void openModel();

public:

    pSourceManager(): debugParse_(false), debugModel_(false), db_(NULL), model_(NULL) { }
    ~pSourceManager();

    void readStubs(pStringRef dirName);

    void setDebug(bool debugParse, bool debugModel) {
        debugParse_ = debugParse;
        debugModel_ = debugModel;
    }

    void printAST();
    void printToks();

    void addSourceFile(pStringRef name);
    void addSourceDir(pStringRef name, pStringRef glob);

    void refreshModel();
    void runDiagnostics();

};

} // namespace

#endif
