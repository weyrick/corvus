/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/passes/TypeAnalysis.h"

namespace corvus { namespace AST { namespace Pass {

void TypeAnalysis::visit_pre_assignment(assignment* n) {

    // if rVal is a literal, we know the type of lVal
    expr* rVal = n->rVal();
    if (llvm::isa<literalExpr>(rVal)) {

        expr* lVal = n->lVal();
        if (llvm::isa<literalNull>(rVal)) {
            lVal->setProp("datatype", "null");
        }
        // XXX more types

    }

    // XXX if it's a constructor call, we know it's an object and the type

}

} } } // namespace

