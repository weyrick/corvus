/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/passes/ModelBuilder.h"

namespace corvus { namespace AST { namespace Pass {


void ModelBuilder::pre_run(void) {

    //scope_.push_back(MODULE);
    m_id_ = model_.getSourceModule(module_->fileName());
    ns_id_ = model_.getNamespace("\\");

}

void ModelBuilder::post_run(void) {

    model_.commit();

}

void ModelBuilder::visit_pre_namespaceDecl(namespaceDecl* n) {

    ns_id_ = model_.getNamespace(n->name());

}


void ModelBuilder::visit_pre_classDecl(classDecl* n) {

    //scope_.push_back(CLASS);

}

void ModelBuilder::visit_post_classDecl(classDecl* n) {

    //scope_.pop_back();

}

void ModelBuilder::visit_pre_signature(signature* n) {


    //scope_.push_back(FUNCTION);

    f_id_list.push_back(model_.defineFunction(ns_id_,
                          m_id_,
                          c_id_,
                          n->name(),
                          pModel::FUNCTION,
                          pModel::NO_FLAGS,
                          pModel::PUBLIC,
                          n->startLineNum(),
                          n->startCol(),
                          n->endLineNum(),
                          n->endCol()
                          ));

    for (int i = 0; i < n->numParams(); i++) {
        formalParam *p = n->getParam(i);
        model_.defineFunctionVar(f_id_list.back(),
                                 p->name(),
                                 pModel::PARAM,
                                 pModel::NO_FLAGS,
                                 pModel::T_UNKNOWN,
                                 "", // XXX datatype obj
                                 "", // XXX default
                                 p->startLineNum(),
                                 p->endLineNum()
                    );
    }

}

void ModelBuilder::visit_post_functionDecl(functionDecl *n) {


    //scope_.pop_back();
    f_id_list.pop_back();

}

/*
void ModelBuilder::visit_pre_block(block* n) {

    // we only push a BLOCK context if the parent is BLOCK
    if (scope_.back() == BLOCK)
        scope_.push_back(BLOCK);

}

void ModelBuilder::visit_post_block(block* n) {

    // we only pop if parent is BLOCK
    if (scope_.back() == BLOCK)
        scope_.pop_back();

}
*/

void ModelBuilder::visit_pre_assignment(assignment* n) {


    expr* lval = n->lVal();
    if (var *i = llvm::dyn_cast<var>(lval)) {
        //do_decl(i->name());
    }

    expr* rval = n->rVal();
    if (var *i = llvm::dyn_cast<var>(rval)) {
        //do_use(i->name());
    }

}

} } } // namespace

