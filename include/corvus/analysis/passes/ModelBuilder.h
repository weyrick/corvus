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

namespace corvus {

class pModelScope;

namespace AST { namespace Pass {

class ModelBuilder: public pBaseVisitor {

private:
    pModelScope *rootNS_;

public:
    ModelBuilder(pModelScope *rootNS):
            pBaseVisitor("NamespaceBuilder","Build the namespace model"),
            rootNS_(rootNS)
            { }

    void pre_run(void);
    /*
    void post_run(void);
    */

    void visit_pre_signature(signature* n);

};

} } } // namespace

#endif
