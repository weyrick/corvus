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

#include <vector>

namespace corvus {

class pSourceManager {

private:

    bool debug_;
    std::vector<pSourceModule*> moduleList_;

public:

    pSourceManager(): debug_() { }
    ~pSourceManager();

    void setDebug(bool debug) { debug_ = debug; }

    void printAST();
    void printToks();

    void addSourceFile(pStringRef name);
    void addSourceDir(pStringRef name, pStringRef glob);

    void refreshModel();

};

} // namespace

#endif
