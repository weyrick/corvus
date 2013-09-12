/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/passes/Trivial.h"
#include <sstream>

namespace corvus { namespace AST { namespace Pass {

/*
void Trivial::pre_run(void) {
}

void Trivial::post_run(void) {
}
*/

void Trivial::visit_pre_signature(signature* n) {



    // verify that if we see a parameter with a default, that
    // we don't find a parameter after it with no default
    // function($foo, $bar='test', $baz)
    int p = n->numParams();
    if (!p)
        return;
    formalParam* param(0);
    formalParam* firstParam(0);
    // note, the params are in reverse order
    for (int i = p-1; i >= 0; i--) {
        param = n->getParam(i);
        if (param->hasDefault()) {
            if (!firstParam)
                firstParam = param;
        }
        else if (firstParam) {
            std::stringstream diag;
            diag << "$" << firstParam->name().str() << " should have explicit default value (currently implicit NULL)";
            addDiagnostic(param, diag.str());
            //addDiagnostic(firstParam, "first parameter with default defined here");
            // return so that we only show the diag once
            return;
        }
    }

}

} } } // namespace

