/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2010 Cornelius Riemenschneider <c.r1@gmx.de>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/pSourceFile.h"
#include "corvus/pPass.h"
#include "corvus/pDiagnostic.h"

namespace corvus { namespace AST {

const char* pPass::nodeDescTable_[] = {
#define STMT(CLASS, PARENT) #CLASS,
#include "corvus/astNodes.def"
};


void pPass::addDiagnostic(AST::stmt* s, pStringRef msg) {
    // the module takes ownership of this
    pDiagnostic *d = new pDiagnostic(s->startLineNum(),
                                     s->startCol(),
                                     s->endLineNum(),
                                     s->endCol(),
                                     msg);
    module_->addDiagnostic(d);
}


} } // namespace

