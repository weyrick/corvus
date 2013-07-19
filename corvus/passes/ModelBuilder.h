/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PASS_MODEL_BUILDER_H_
#define COR_PASS_MODEL_BUILDER_H_

#include "corvus/pAST.h"
#include "corvus/pBaseVisitor.h"
#include "corvus/pModel.h"

#include <vector>

namespace corvus {

namespace AST { namespace Pass {

class ModelBuilder: public pBaseVisitor {

private:

    pModel::oid m_id_;
    pModel::oid ns_id_;
    pModel::oid c_id_;
    std::vector<pModel::oid> f_id_list;

public:
    ModelBuilder():
            pBaseVisitor("ModelBuilder","Build the code model"),
            c_id_(pModel::NULLID)
            { }

    void pre_run(void);
    void post_run(void);

    void visit_pre_namespaceDecl(namespaceDecl* n);
    void visit_post_namespaceDecl(namespaceDecl* n);

    void visit_pre_classDecl(classDecl* n);
    void visit_post_classDecl(classDecl* n);
    void visit_post_propertyDecl(propertyDecl *n);

    void visit_pre_signature(signature* n);    
    void visit_post_functionDecl(functionDecl *n);

    void visit_pre_functionInvoke(functionInvoke *n);

    /*
    void visit_pre_block(block* n);
    void visit_post_block(block* n);
    */

    void visit_pre_assignment(assignment* n);

};

} } } // namespace

#endif
