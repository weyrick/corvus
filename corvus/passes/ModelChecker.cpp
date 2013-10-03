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
    pNSVisitor::pre_run();
    m_id_ = model_->getSourceModuleOID(module_->fileName());
    assert(m_id_ != pModel::NULLID && "module not found");
}

/*
void ModelChecker::post_run(void) {
}
*/

void ModelChecker::visit_pre_classDecl(classDecl* n) {

    c_id_ = model_->lookupClass(ns_id_, n->name(), m_id_);
    if (c_id_ == pModel::NULLID)
        std::cout << "no class found: " << n->name().str() << " ns_id: " << ns_id_ << " for line " << n->range().startLine << " in " << module_->fileName() << "\n";
    assert(c_id_ != pModel::NULLID && "class wasn't in the model");

}

void ModelChecker::visit_post_classDecl(classDecl* n) {

    c_id_ = pModel::NULLID;

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
    std::pair<pModel::oid, std::string> resolved = model_->resolveFQN(ns_id_, RESOLVE_FQN(n->literalName().str()));
    pModel::FunctionList list = model_->queryFunctions(resolved.first, c_id, resolved.second);

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
        pModel::ConstantList cn = model_->queryConstants(RESOLVE_FQN(n->name().str()), ns_id_);
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

        pModel::oid class_id;
        if (classID->name() == "self") {
            class_id = c_id_;
        }
        else {
            class_id = model_->lookupClass(ns_id_, RESOLVE_FQN(classID->name().str()));
            if (class_id == pModel::NULLID) {
                std::stringstream diag;
                diag << "class constant from undefined class: " << classID->name().str();
                addDiagnostic(target, diag.str());
                return;
            }
            else if (class_id == pModel::MULTIPLE_IDS) {
                std::stringstream diag;
                diag << "class constant from class defined multiple times: " << classID->name().str();
                addDiagnostic(target, diag.str());
                return;
            }
        }

        pModel::ClassDeclList cdl = model_->queryClassDecls(class_id, n->name());
        if (cdl.size() == 0) {
            std::stringstream diag;
            diag << "undefined class constant: " << classID->name().str() << "::" << n->name().str();
            addDiagnostic(target, diag.str());
            return;
        }
    }

}

} } } // namespace

