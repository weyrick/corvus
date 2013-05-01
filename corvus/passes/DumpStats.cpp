/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2010 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/passes/DumpStats.h"
#include "corvus/pSourceModule.h"

#include <iostream>

namespace corvus { namespace AST { namespace Pass {


void DumpStats::post_run(void) {

    module_->dumpContextStats();

}


} } } // namespace

