/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/pConfig.h"

#include <iostream>

#include <llvm/ADT/OwningPtr.h>
#include <llvm/Value.h> // isa
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLParser.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/system_error.h>

namespace corvus { 

bool pConfig::read(const llvm::Twine &file) {

    llvm::SmallString<128> path_storage;
    llvm::StringRef fName = file.toStringRef(path_storage);

    llvm::OwningPtr<llvm::MemoryBuffer> contents;
    if (llvm::MemoryBuffer::getFile(fName, contents)) {
        return false;
    }

    llvm::SourceMgr sm;
    llvm::StringRef input = contents->getBufferStart();
    llvm::yaml::Stream stream(input, sm);

    for (llvm::yaml::document_iterator di = stream.begin(), de = stream.end();
       di != de; ++di) {
        llvm::yaml::Node *n = di->getRoot();
        if (n) {
        }
        else {
          break;
        }
    }

    return true;

}

} // namespace

