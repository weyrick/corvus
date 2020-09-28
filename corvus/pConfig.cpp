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

#if (LLVM_VERSION >= 3003000)
#include <llvm/IR/Value.h> // isa
#else
#include <llvm/Value.h>
#endif

#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLParser.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/APInt.h>

#include <system_error>                 // for error_code

namespace corvus { 

bool pConfigMgr::read(const llvm::Twine &file, pConfig &c) {

    llvm::SmallString<128> path_storage;
    llvm::StringRef fName = file.toStringRef(path_storage);

    std::unique_ptr<llvm::MemoryBuffer> contents;
    if (llvm::MemoryBuffer::getFile(fName)) {
        return false;
    }

    llvm::SourceMgr sm;
    llvm::StringRef input = contents->getBufferStart();
    llvm::yaml::Stream stream(input, sm);
    llvm::SmallString<128> kstorage;
    llvm::SmallString<128> vstorage;

    for (llvm::yaml::document_iterator di = stream.begin(), de = stream.end();
       di != de; ++di) {
        llvm::yaml::MappingNode *n = llvm::dyn_cast<llvm::yaml::MappingNode>(di->getRoot());
        if (!n)
            break;
        for (llvm::yaml::MappingNode::iterator i = n->begin();
             i != n->end();
             ++i) {
            llvm::yaml::ScalarNode *keyN = llvm::dyn_cast<llvm::yaml::ScalarNode>(i->getKey());
            llvm::yaml::ScalarNode *valN = llvm::dyn_cast<llvm::yaml::ScalarNode>(i->getValue());
            if (!keyN || !valN)
                continue;
            //std::cout << "key is " << key->getValue(storage).str() << ", val is " << val->getValue(storage).str() << std::endl;

            pStringRef key = keyN->getValue(kstorage);
            pStringRef val = valN->getValue(vstorage);
            if (key == "include") {
                c.includePaths.push_back(val.str());
            }
            else if (key == "source_directory" || key == "source_file") {
                c.inputFiles.push_back(val.str());
            }
            else if (key == "diagnostic_file") {
                c.diagFiles.push_back(val.str());
            }
            else if (key == "db") {
                c.dbName = val.str();
            }
            else if (key == "exts") {
                c.exts = val.str();
            }
            else if (key == "verbosity") {
                llvm::APInt result;
                val.getAsInteger(10, result);
                c.verbosity = result.getLimitedValue();
            }
            else {
                std::cerr << "unknown key in config file: " << key.str() << std::endl;
            }

        }
    }

    return true;

}

} // namespace

