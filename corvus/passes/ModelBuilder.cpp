/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "md5.h"

#include "corvus/passes/ModelBuilder.h"

#include "corvus/pSourceModule.h"
#include "corvus/pSourceFile.h"
#include <stdio.h>

namespace corvus { namespace AST { namespace Pass {


void ModelBuilder::pre_run(void) {

    // generate the source hash
    md5_byte_t digest[16];
    md5_state_t state;
    md5_init(&state);
    md5_append(&state,
               reinterpret_cast<const md5_byte_t *>(module_->source()->contents()->getBufferStart()),
               module_->source()->contents()->getBufferSize());
    md5_finish(&state, digest);
    char hash[32];
    for (int di = 0; di < 16; ++di)
        sprintf(hash + di * 2, "%02x", digest[di]);

    // is the source module dirty? i.e. does it exist in the model already and
    // has it changed since we last built it?
    if (!model_->sourceModuleDirty(module_->fileName(), hash)) {
        // don't run the pass
        abortPass();
        return;
    }

    m_id_ = model_->getSourceModuleOID(module_->fileName(),
                                       hash,
                                       true /* delete first */
                                       );
    ns_id_ = model_->getRootNamespaceOID();

}

void ModelBuilder::post_run(void) {

    model_->commit();

}

void ModelBuilder::visit_pre_namespaceDecl(namespaceDecl* n) {

    ns_id_ = model_->getNamespaceOID(n->name(), true);

}

void ModelBuilder::visit_post_namespaceDecl(namespaceDecl* n) {

    // we only lose the namespace if this one had a body, i.e. block
    if (n->body())
        ns_id_ = model_->getRootNamespaceOID();

}

void ModelBuilder::visit_pre_classDecl(classDecl* n) {

    std::stringstream extends, implements;
    if (n->extendsCount()) {
        for (idList::iterator i = n->extends_begin();
             i != n->extends_end();
             ++i) {
            // php has no multiple inheritance, so we expect only 1
            extends << *i << ",";
        }
    }
    if (n->implementsCount()) {
        for (idList::iterator i = n->implements_begin();
             i != n->implements_end();
             ++i) {
            implements << *i << ",";
        }
    }

    c_id_ = model_->defineClass(ns_id_,
                                m_id_,
                                n->name(),
                                (n->classType() == classDecl::IFACE) ?
                                    pModel::IFACE : pModel::CLASS,
                                extends.str(),
                                implements.str(),
                                n->range());

}

void ModelBuilder::visit_post_propertyDecl(propertyDecl *n) {

    if (n->flags() & memberFlags::CONST) {
        expr* value = n->defaultValue();
        assert(llvm::isa<literalExpr>(value) || llvm::isa<unaryOp>(value));        
        std::string def;
        if (llvm::isa<unaryOp>(value)) {
            switch (llvm::dyn_cast<unaryOp>(value)->opKind()) {
                case unaryOp::NEGATIVE:
                    def.push_back('-');
                    break;
                case unaryOp::BITWISENOT:
                    def.push_back('~');
                    break;
            }
            value = llvm::dyn_cast<unaryOp>(value)->rVal();
            def.append(llvm::dyn_cast<literalExpr>(value)->getStringVal());
        }
        else {
            def = llvm::dyn_cast<literalExpr>(value)->getStringVal();
        }
        model_->defineClassDecl(c_id_,
                                n->name(),
                                pModel::CONST,
                                pModel::NO_FLAGS,
                                pModel::PUBLIC, // implicit
                                def,
                                n->range());
    }

}

void ModelBuilder::visit_post_classDecl(classDecl* n) {

    c_id_ = pModel::NULLID;

}

void ModelBuilder::visit_pre_signature(signature* n) {


    int minArity = 0;
    for (int i = 0; i < n->numParams(); i++) {
        formalParam *p = n->getParam(i);
        if (p->hasDefault())
            break;
        minArity++;
    }
    f_id_list.push_back(model_->defineFunction(ns_id_,
                          m_id_,
                          c_id_,
                          n->name(),
                          pModel::FUNCTION,
                          pModel::NO_FLAGS,
                          pModel::PUBLIC,
                          minArity,
                          n->numParams(),
                          n->range()
                          ));

    for (int i = 0; i < n->numParams(); i++) {
        formalParam *p = n->getParam(i);
        model_->defineFunctionVar(f_id_list.back(),
                                 p->name(),
                                 pModel::PARAM,
                                 pModel::NO_FLAGS,
                                 pModel::T_UNKNOWN,
                                 "", // XXX datatype obj
                                 "", // XXX default
                                 p->range()
                    );
    }

}

void ModelBuilder::visit_post_functionDecl(functionDecl *n) {


    f_id_list.pop_back();

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

void ModelBuilder::visit_pre_functionInvoke(functionInvoke *n) {

    // can't do anything with dynamics
    if (!n->hasLiteralName())
        return;

    if (n->target()) {
        // method invoke
    }
    else {
        // function invoke

        // if this is define(), we do a constant
        if (n->literalName().equals("define") && n->numArgs() == 2) {
            // need to pull the name and value from param list
            expr* name = n->arg(0);
            expr* value = n->arg(1);
            if (!llvm::isa<literalExpr>(name)) {
                addDiagnostic(n, "expected literal name for define()");
                return;
            }
            std::string strval;
            if (llvm::isa<literalExpr>(value)) {
                strval = llvm::dyn_cast<literalExpr>(value)->getStringVal();
            }
            model_->defineConstant(m_id_,
                                   llvm::dyn_cast<literalExpr>(name)->getStringVal(),
                                   pModel::DEFINE,
                                   strval,
                                   n->range());
            return;
        }
    }

}

} } } // namespace

