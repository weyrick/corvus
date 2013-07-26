/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "pFullModelChecker.h"
#include "pModel.h"
#include "pSourceManager.h"
#include "pDiagnostic.h"
#include <sstream>

namespace corvus {

void pFullModelChecker::addDiagnostic(pStringRef realPath, int sl, int sc, pStringRef msg) {
    pSourceModule *sm = sourceMgr_->getSourceModuleByRealpath(realPath);
    assert(sm && "lost source module");
    sm->addDiagnostic(new pDiagnostic(pSourceLoc(sm, sl, sc), msg));
}

void pFullModelChecker::run() {

    // make sure all classes are resolved (extends and implements)
    pModel::ClassList unresolved = model_->getUnresolvedClasses();
    for (int i = 0; i < unresolved.size(); ++i) {
        if (unresolved[i].getAsInt("extends_count") > unresolved[i].getAsInt("resolved_extends_count")) {
            std::stringstream diag;
            diag << "class " << unresolved[i].get("name") << " extends " << unresolved[i].get("extends") << " which is unresolved";
            addDiagnostic(unresolved[i].get("realpath"),
                          unresolved[i].getAsInt("start_line"),
                          unresolved[i].getAsInt("start_col"),
                          diag.str());
        }
        if (unresolved[i].getAsInt("implements_count") > unresolved[i].getAsInt("resolved_implements_count")) {
            std::stringstream diag;
            diag << "class " << unresolved[i].get("name") << " implements " << unresolved[i].get("implements") << " which is unresolved";
            addDiagnostic(unresolved[i].get("realpath"),
                          unresolved[i].getAsInt("start_line"),
                          unresolved[i].getAsInt("start_col"),
                          diag.str());
        }
    }

    // class relations check: make sure classes listed in "implements" are all
    // interfaces, not classes


}


} // namespace
