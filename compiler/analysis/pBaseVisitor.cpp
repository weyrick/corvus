/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/pBaseVisitor.h"
#include "corvus/analysis/pSourceModule.h"


#include <iostream>

namespace corvus { namespace AST {

pBaseVisitor::dispatchFunction pBaseVisitor::preDispatchTable_[] = {

#define STMT(CLASS, PARENT) reinterpret_cast<dispatchFunction>( &pBaseVisitor::visit_pre_##CLASS ),
#include "corvus/analysis/astNodes.def"

};

pBaseVisitor::dispatchFunction pBaseVisitor::postDispatchTable_[] = {

#define STMT(CLASS, PARENT) reinterpret_cast<dispatchFunction>( &pBaseVisitor::visit_post_##CLASS ),
#include "corvus/analysis/astNodes.def"

};

pBaseVisitor::childDispatchFunction pBaseVisitor::childrenDispatchTable_[] = {

#define STMT(CLASS, PARENT) reinterpret_cast<childDispatchFunction>( &pBaseVisitor::visit_children_##CLASS ),
#include "corvus/analysis/astNodes.def"

};

void pBaseVisitor::run(void) {
    module_->applyVisitor(this);
}

void pBaseVisitor::visit(stmt* s) {

    // PRE
    visit_pre_stmt(s);
    if (expr* n = dyn_cast<expr>(s))
        visit_pre_expr(n);

    (this->*preDispatchTable_[s->kind()])(s);

    // CHILDREN
    // we always try the custom first, and fall back to the standard unless
    // the custom returns true
    if ((this->*childrenDispatchTable_[s->kind()])(s) == false)
        visitChildren(s);

    // POST
    (this->*postDispatchTable_[s->kind()])(s);

    if (expr* n = dyn_cast<expr>(s))
        visit_post_expr(n);
    visit_post_stmt(s);

}

void pBaseVisitor::visitChildren(stmt* s) {

    stmt* child(0);
    for (stmt::child_iterator i = s->child_begin(), e = s->child_end(); i != e; ) {
      if ( (child = *i++) ) {
          visit(child);
      }
    }

}

void pBaseVisitor::visitChildren(stmt::child_iterator begin, stmt::child_iterator end) {

    stmt* child(0);
    for (stmt::child_iterator i = begin, e = end; i != e; ) {
      if ( (child = *i++) ) {
          visit(child);
      }
    }

}

} } // namespace

