/* ***** BEGIN LICENSE BLOCK *****
;; corvus analyzer
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This program is free software; you can redistribute it and/or
;; modify it under the terms of the GNU General Public License
;; as published by the Free Software Foundation; either version 2
;; of the License, or (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, write to the Free Software
;; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PASS_TRIVIAL_H_
#define COR_PASS_TRIVIAL_H_

#include "corvus/analysis/pAST.h"
#include "corvus/analysis/pBaseVisitor.h"

namespace corvus { namespace AST { namespace Pass {

class Trivial: public pBaseVisitor {

    void doComment(const char*);
    void doNullChild(void);

    void visitOrNullChild(stmt*);

public:
    Trivial(pSourceModule* m):
            pBaseVisitor("Trivial","Trivial static checks requiring a single pass", m)
            { }

    /*
    void pre_run(void);
    void post_run(void);
    */

    void visit_pre_signature(signature* n);

};

} } } // namespace

#endif
