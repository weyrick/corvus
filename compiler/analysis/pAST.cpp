/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2009-2010 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
;;
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/pAST.h"

namespace corvus { namespace AST {

void stmt::destroyChildren(pParseContext &C) {
    for (child_iterator i = child_begin(), e = child_end(); i != e; ) {
        // NOTE: it's valid for some children to be NULL
        if (stmt* child = *i++)
            child->destroy(C);
    }
}

void stmt::doDestroy(pParseContext &C) {
  destroyChildren(C);
  this->~stmt();
  C.deallocate((void *)this);

}

const pUInt memberFlags::PUBLIC    = 1;
const pUInt memberFlags::PROTECTED = 2;
const pUInt memberFlags::PRIVATE   = 4;
const pUInt memberFlags::STATIC    = 8;
const pUInt memberFlags::ABSTRACT  = 16;
const pUInt memberFlags::FINAL     = 32;
const pUInt memberFlags::CONST     = 64;

} } // namespace
