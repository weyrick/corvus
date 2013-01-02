/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_DUMPAST_H_
#define COR_DUMPAST_H_

#include "corvus/analysis/pAST.h"
#include "corvus/analysis/pBaseVisitor.h"

class TiXmlDocument;
class TiXmlElement;

namespace corvus { namespace AST { namespace Pass {

class PrintAST: public pBaseVisitor {

    TiXmlDocument*doc_;
    TiXmlElement* currentElement_;

    void doComment(const char*);
    void doNullChild(void);

    void visitOrNullChild(stmt*);

public:
    PrintAST():
            pBaseVisitor("AST Dump","Basic dump of the AST"),
            doc_(NULL),
            currentElement_(NULL)
            { }

    void pre_run(void);
    void post_run(void);

    void visit_pre_stmt(stmt*);
    void visit_post_stmt(stmt*);

    void visit_pre_signature(signature* n);
    void visit_pre_formalParam(formalParam* n);
    bool visit_children_formalParam(formalParam* n);
    void visit_pre_classDecl(classDecl* n);
    void visit_pre_methodDecl(methodDecl* n);
    bool visit_children_methodDecl(methodDecl* n);
    void visit_pre_propertyDecl(propertyDecl* n);

    bool visit_children_staticDecl(staticDecl* n);

    void visit_pre_assignment(assignment* n);
    void visit_pre_builtin(builtin* n);
    void visit_pre_literalID(literalID* n);
    void visit_pre_var(var* n);
    void visit_pre_unaryOp(unaryOp* n);
    void visit_pre_binaryOp(binaryOp* n);
    void visit_pre_literalString(literalString* n);
    void visit_pre_inlineHtml(inlineHtml* n);
    void visit_pre_literalInt(literalInt* n);
    void visit_pre_literalFloat(literalFloat* n);
    void visit_pre_literalBool(literalBool* n);
    void visit_pre_literalArray(literalArray* n);
    void visit_pre_literalConstant(literalConstant* n);
    void visit_pre_label(label* n);
    void visit_pre_branch(branch* n);
    void visit_pre_typeCast(typeCast* n);


};

} } } // namespace

#endif /* COR_DUMPAST_H_ */
