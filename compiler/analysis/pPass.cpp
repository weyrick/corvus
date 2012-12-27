/* ***** BEGIN LICENSE BLOCK *****
;; corvus analyzer
;;
;; Copyright (c) 2010 Cornelius Riemenschneider <c.r1@gmx.de>
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

