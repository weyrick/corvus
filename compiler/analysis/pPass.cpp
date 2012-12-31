/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2010 Cornelius Riemenschneider <c.r1@gmx.de>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/pSourceFile.h"
#include "corvus/analysis/pPass.h"

namespace corvus { namespace AST {

const char* pPass::nodeDescTable_[] = {
#define STMT(CLASS, PARENT) #CLASS,
#include "corvus/analysis/astNodes.def"
};


void pPass::addDiagnostic(AST::stmt* s, pStringRef msg) {
    // XXX this is temporary
    std::cout << C_.getOwner()->fileName() << ":" << s->startLineNum() << ":" << s->startCol() << ": " << msg.data() << std::endl;
    // diag source line
    pSourceCharIterator i = C_.getOwner()->source()->contents()->getBufferStart();
    pSourceCharIterator end = C_.getOwner()->source()->contents()->getBufferEnd();
    pUInt curLine(1), diagLine(s->startLineNum());
    while (curLine != diagLine) {
        if (*i == '\n')
            curLine++;
        i++;
    }
    pSourceCharIterator e = i;
    while (*e != '\n' && e != end)
        e++;
    pStringRef line(i, e-i);
    std::cout << line.str() << std::endl;
    // arrow to problem column
    std::cout << std::string(s->startCol()-1, ' ') << "^" << std::endl;
}


} } // namespace

