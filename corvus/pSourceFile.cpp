/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/pSourceFile.h"
#include "corvus/pParseError.h"
#include "corvus/pSourceLoc.h"

#include <system_error>                 // for error_code

namespace corvus { 

pSourceFile::pSourceFile(pStringRef file):
    file_(file)
{

    if (llvm::MemoryBuffer::getFile(file)) {
        throw pParseError("couldn't open file [" + file_ + "]", pSourceLoc(file, 0, 0));
    }
    
}


} // namespace

