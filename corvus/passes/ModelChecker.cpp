/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/passes/ModelChecker.h"
#include <sstream>

namespace corvus { namespace AST { namespace Pass {


void ModelChecker::pre_run(void) {
    ns_id_ = model_->getNamespaceOID("\\");
}

/*
void ModelChecker::post_run(void) {
}
*/

void ModelChecker::visit_pre_namespaceDecl(namespaceDecl* n) {

    ns_id_ = model_->getNamespaceOID(n->name());

}

void ModelChecker::visit_pre_functionInvoke(functionInvoke* n) {

    // can't do anything with dynamic function invokes, i.e. $foo()
    if (!n->hasLiteralName())
        return;

    // constructor?
    if (n->constructor()) {
        // XXX check that the class exists and check constructor args
        return;
    }

    // method?
    pModel::oid c_id = pModel::NULLID;
    if (n->target()) {
        if (n->hasLiteralTarget()) {
            // XXX static call: get class oid
            return;
        }
        else {
            // a dynamic method call. we can't diag this without type analysis
            return;
        }
    }

    // find the function in the model
    pModel::FunctionList list = model_->queryFunctions(ns_id_, c_id, n->literalName());

    // if it doesn't exist, diag it
    std::stringstream diag;

    if (list.size() == 0) {
        diag << "function '" << n->literalName().str() << "' not defined";
        addDiagnostic(n, diag.str());
        return;
    }
    // if there are multiple hits, diag it
    else if (list.size() > 1) {
        diag << "function '" << n->literalName().str() << "' defined in multiple locations:";
        addDiagnostic(n, diag.str());
        for (int i = 0; i < list.size(); i++) {
            std::cout << "\t" << list[i].sourceModule << ":" << list[i].startLine << std::endl;
        }
        return;
    }

    // one hit, check arity
    pUInt arity = n->numArgs();
    if (arity < list[0].minArity || arity > list[0].maxArity) {
        if (list[0].minArity == list[0].maxArity) {
            diag << "wrong number of arguments: function '" << n->literalName().str()
                 << "' requires " << list[0].minArity << " arguments (" << arity << " specified)";
        }
        else {
            diag << "wrong number of arguments: function '" << n->literalName().str()
                 << "' takes between " << list[0].minArity << " and "
                 << list[0].maxArity << " arguments (" << arity << " specified)";
        }
        addDiagnostic(n, diag.str());
    }

}

} } } // namespace

