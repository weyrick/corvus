/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/passes/ModelBuilder.h"

namespace corvus { namespace AST { namespace Pass {


void ModelBuilder::pre_run(void) {

    //module_->fileName();

}

/*
void NamespaceBuilder::post_run(void) {
}
*/

void ModelBuilder::visit_pre_signature(signature* n) {


    std::cout << "building: " << n->name().str() << "\n";

}

} } } // namespace

