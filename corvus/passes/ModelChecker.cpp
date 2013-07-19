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
        diag << "function '" << n->literalName().str() << "' defined in " << list.size() << " locations";
        addDiagnostic(n, diag.str());
        int max = list.size();
        if (max > 3)
            max = 3;
        for (int i = 0; i < max; i++) {
            diag << list[i].get("realpath") << ":" << list[i].get("start_line") << std::endl;
        }
        return;
    }

    // one hit, check arity
    pUInt arity = n->numArgs();
    if (arity < list[0].getAsInt("minArity") || arity > list[0].getAsInt("maxArity")) {
        if (list[0].getAsInt("minArity") == list[0].getAsInt("maxArity")) {
            diag << "wrong number of arguments: function '" << n->literalName().str()
                 << "' requires " << list[0].get("minArity") << " arguments (" << arity << " specified)";
        }
        else {
            diag << "wrong number of arguments: function '" << n->literalName().str()
                 << "' takes between " << list[0].get("minArity") << " and "
                 << list[0].get("maxArity") << " arguments (" << arity << " specified)";
        }
        addDiagnostic(n, diag.str());
    }

}

void ModelChecker::visit_pre_literalConstant(literalConstant* n) {

    // make sure this was define()'d
    if (!n->target()) {
        pModel::ConstantList cn = model_->queryConstants(n->name());
        if (cn.size() == 0) {
            std::stringstream diag;
            diag << "undefined constant: " << n->name().str();
            addDiagnostic(n, diag.str());
            return;
        }
    }
    else {
        // class constant
        expr* target = n->target();
        if (!llvm::isa<literalID>(target))
            // dynamic class name
            return;
        literalID* classID = llvm::dyn_cast<literalID>(target);
        pModel::ClassList cl = model_->queryClasses(ns_id_, classID->name());
        if (cl.size() > 1) {
            std::stringstream diag;
            // XXX FQN
            diag << "class redefined: " << classID->name().str();
            addDiagnostic(target, diag.str());
            return;
        }
        else if (cl.size() == 0) {
            std::stringstream diag;
            // XXX FQN
            diag << "undefined class: " << classID->name().str();
            addDiagnostic(target, diag.str());
            return;
        }
        pModel::ClassDeclList cdl = model_->queryClassDecls(cl[0].getID(), n->name());
        if (cdl.size() == 0) {
            std::stringstream diag;
            // XXX FQN
            diag << "undefined class constant: " << classID->name().str() << "::" << n->name().str();
            addDiagnostic(target, diag.str());
            return;
        }
    }

}

} } } // namespace

