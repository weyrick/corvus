/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PSOURCEFILE_H_
#define COR_PSOURCEFILE_H_

#include "corvus/pTypes.h"

#include <string>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/MemoryBuffer.h>

using llvm::MemoryBuffer;

namespace corvus {

class pSourceFile {

private:
    std::string file_;
    std::unique_ptr<MemoryBuffer> contents_;

public:

    pSourceFile(pStringRef file);

    const std::string& fileName(void) const {
        return file_;
    }
    const MemoryBuffer* contents(void) const { return contents_.get(); }

};

} // namespace

#endif /* COR_PSOURCEFILE_H_ */
