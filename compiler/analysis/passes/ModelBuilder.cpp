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

    scope_.push_back(MODULE);
    module_id_ = model_.getSourceModule(module_->fileName());
    namespace_id_ = model_.getNamespace("\\");

}

void ModelBuilder::post_run(void) {

}

void ModelBuilder::visit_pre_namespaceDecl(namespaceDecl* n) {

    namespace_id_ = model_.getNamespace(n->name());

}


void ModelBuilder::visit_pre_classDecl(classDecl* n) {

    scope_.push_back(CLASS);

    //do_decl(n->name().str());

}

void ModelBuilder::visit_post_classDecl(classDecl* n) {

    scope_.pop_back();

}

void ModelBuilder::visit_pre_signature(signature* n) {


    scope_.push_back(FUNCTION);

    pModel::oid parent_id = 0;
    if (context_.size())
        parent_id = context_.back();
/*
    pModel::oid c = model_.getContext(module_id_,
                                      parent_id,
                                      scope_.back(),
                                      n->startLineNum(),
                                      n->startCol(),
                                      n->endLineNum(),
                                      n->endCol());
    context_.push_back(c);
*/
    for (int i = 0; i < n->numParams(); i++) {
        //do_decl(n->getParam(i)->name());
    }

}

void ModelBuilder::visit_post_signature(signature* n) {


    scope_.pop_back();

}

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

