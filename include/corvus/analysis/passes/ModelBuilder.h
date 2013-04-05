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

#include "corvus/analysis/pAST.h"
#include "corvus/analysis/pBaseVisitor.h"
#include "corvus/analysis/pModel.h"

#include <vector>

namespace corvus {

namespace AST { namespace Pass {

class ModelBuilder: public pBaseVisitor {
public:
    enum kind { MODULE, CLASS, FUNCTION, BLOCK };

private:
    std::vector<kind> scope_;

    void do_decl(const std::string& name);
    void do_use(const std::string& name);

    pModel &model_;

    pModel::oid module_id_;

    std::string namespace_;

public:
    ModelBuilder(pModel& model):
            pBaseVisitor("ModelBuilder","Build the code model"),
            model_(model),
            scope_()
            { }

    void pre_run(void);
    void post_run(void);

    void visit_pre_namespaceDecl(namespaceDecl* n);

    void visit_pre_classDecl(classDecl* n);
    void visit_post_classDecl(classDecl* n);

    void visit_pre_signature(signature* n);
    void visit_post_signature(signature* n);

    void visit_pre_block(block* n);
    void visit_post_block(block* n);

    void visit_pre_assignment(assignment* n);

};

} } } // namespace

#endif
