/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/passes/ModelChecker.h"
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

    // can't do anything with dynamic function invokes
    if (n->isDynamic())
        return;

    // method?
    pModel::oid c_id = pModel::NULLID;
    if (n->hasLiteralTarget()) {
        // XXX get class oid
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
            std::cout << "\t" << list[i].sourceModule << ":" << list[i].startCol << std::endl;
        }
        return;
    }

    // one hit, check arity
    pUInt arity = n->numArgs();
    if (arity < list[0].minArity || arity > list[0].maxArity) {
        diag << "wrong number of arguments: function '" << n->literalName().str()
             << "' takes between " << list[0].minArity << " and "
             << list[0].maxArity << " arguments (" << arity << " are specified)";
        addDiagnostic(n, diag.str());
    }

}

} } } // namespace

