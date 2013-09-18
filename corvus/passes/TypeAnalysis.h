/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PASS_TYPEANALYSIS_H_
#define COR_PASS_TYPEANALYSIS_H_

#include "corvus/pAST.h"
#include "corvus/pBaseVisitor.h"

namespace corvus { namespace AST { namespace Pass {

class TypeAnalysis: public pBaseVisitor {

public:
    TypeAnalysis():
            pBaseVisitor("TypeAnalysis","Do simple type analysis")
            { }

    /*
    void pre_run(void);
    void post_run(void);
    */

    void visit_pre_assignment(assignment* n);

};

} } } // namespace

#endif
