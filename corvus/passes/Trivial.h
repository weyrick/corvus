/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PASS_TRIVIAL_H_
#define COR_PASS_TRIVIAL_H_

#include "corvus/pAST.h"
#include "corvus/pBaseVisitor.h"

namespace corvus { namespace AST { namespace Pass {

class Trivial: public pBaseVisitor {

    void doComment(const char*);
    void doNullChild(void);

    void visitOrNullChild(stmt*);

public:
    Trivial():
            pBaseVisitor("Trivial","Trivial static checks requiring a single pass")
            { }

    /*
    void pre_run(void);
    void post_run(void);
    */

    void visit_pre_signature(signature* n);

};

} } } // namespace

#endif
