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

    classRelations();
    declUse();

}

void pFullModelChecker::declUse() {

    // diag any uses which had no decl
    pModel::UndeclList undecl = model_->getUndeclaredUses();
    for (int i = 0; i < undecl.size(); ++i) {
        std::stringstream diag;
        diag << "$" << undecl[i].get("name") << " used but not defined";
        addDiagnostic(undecl[i].get("realpath"),
                      undecl[i].getAsInt("start_line"),
                      undecl[i].getAsInt("start_col"),
                      diag.str()
                    );
    }

}

void pFullModelChecker::classRelations() {

    // make sure all classes are resolved (extends and implements)
    pModel::ClassList unresolved = model_->getUnresolvedClasses();
    for (int i = 0; i < unresolved.size(); ++i) {
        std::string unresolved_extends = unresolved[i].get("unresolved_extends");
        std::string unresolved_implements = unresolved[i].get("unresolved_implements");
        if (unresolved_extends.size()) {
            std::stringstream diag;
            diag << "class " << unresolved[i].get("name") << " extends " << unresolved_extends << " which ";
            if (unresolved_extends.find(',') != std::string::npos)
                diag << "are ";
            else
                diag << "is ";
            diag << "unresolved";
            addDiagnostic(unresolved[i].get("realpath"),
                          unresolved[i].getAsInt("start_line"),
                          unresolved[i].getAsInt("start_col"),
                          diag.str());
        }
        if (unresolved_implements.size()) {
            std::stringstream diag;
            diag << "class " << unresolved[i].get("name") << " implements " << unresolved_implements << " which ";
            if (unresolved_implements.find(',') != std::string::npos)
                diag << "are ";
            else
                diag << "is ";
            diag << "unresolved";
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
