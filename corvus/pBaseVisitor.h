/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PBASEVISITOR_H_
#define COR_PBASEVISITOR_H_

#include "corvus/pAST.h"
#include "corvus/pPass.h"

namespace corvus { namespace AST {

class pBaseVisitor: public pPass {
private:

    typedef void (pBaseVisitor::*dispatchFunction)(stmt *);
    typedef bool (pBaseVisitor::*childDispatchFunction)(stmt *);

    static dispatchFunction preDispatchTable_[];
    static dispatchFunction postDispatchTable_[];
    static childDispatchFunction childrenDispatchTable_[];

public:
    pBaseVisitor(const char* name, const char* desc): pPass(name,desc) { }
    virtual ~pBaseVisitor(void) { }

    // pass
    void run(void);

    // root dispatch
    void visit(stmt*);
    virtual void visitChildren(stmt*);
    virtual void visitChildren(stmt::child_iterator begin, stmt::child_iterator end);

    virtual void visit_pre_stmt(stmt* ) { }
    virtual void visit_post_stmt(stmt* ) { }

    virtual void visit_pre_expr(expr* ) { }
    virtual void visit_post_expr(expr* ) { }

    // PRE
#define STMT(CLASS, PARENT) virtual void visit_pre_##CLASS(CLASS *) { }
#include "corvus/astNodes.def"

    // POST
#define STMT(CLASS, PARENT) virtual void visit_post_##CLASS(CLASS *) { }
#include "corvus/astNodes.def"

    // CHILDREN
    // for custom children handler, define and return true
#define STMT(CLASS, PARENT) virtual bool visit_children_##CLASS(CLASS *) { return false; }
#include "corvus/astNodes.def"

};


} } // namespace

#endif /* COR_PBASEVISITOR_H_ */
