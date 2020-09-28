/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2010 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PPARSECONTEXT_H_
#define COR_PPARSECONTEXT_H_

#include "corvus/pTypes.h"

#include <llvm/Support/Allocator.h>
#include <llvm/Support/StringPool.h>
#include <boost/unordered_map.hpp>


namespace corvus {

class pSourceModule;

namespace AST {
class pParseContext {
private:
    typedef boost::unordered_map<const pSourceRef*, pUInt> lineNumMapType;

    pUInt currentLineNum_;
    pSourceCharIterator lastNewline_;
    const pSourceRef* lastToken_;
    lineNumMapType tokenLineInfo_;

    /// Maintains memory of IR during entire analysis and code gen phases
    llvm::BumpPtrAllocator allocator_;

    /// String pool for identifiers to use. Also lives through analysis
    llvm::StringPool idPool_;

    // owning source module
    const pSourceModule* owner_;

public:

    pParseContext(const pSourceModule* o):
        currentLineNum_(0),
        lastNewline_(),
        lastToken_(NULL),
        tokenLineInfo_(),
        allocator_(),
        idPool_(),
        owner_(o)
        { }

    // MEMORY POOL
    llvm::BumpPtrAllocator& allocator(void) { return allocator_; }
    void *allocate(size_t size, size_t align = 8) {
        return allocator_.Allocate(size, align);
    }
    void deallocate(void* Ptr, size_t Size = 4096) {
        // note this is a NOOP for bumpptr
        allocator_.Deallocate(Ptr, Size);
    }

    llvm::StringPool& idPool(void) { return idPool_; }

    // PARSING
    pSourceRange currentLineNum() const { return pSourceRange(currentLineNum_); }
    void incLineNum(void) { ++currentLineNum_; }
    void incLineNum(pUInt i) { currentLineNum_ +=i; }

    pSourceRange getRange(pSourceRef* r);

    void setLastNewline(pSourceCharIterator i) { lastNewline_ = i; }
    const pSourceCharIterator& lastNewline(void) const { return lastNewline_; }

    void setLastToken(const pSourceRef* i) { lastToken_ = i; }
    const pSourceRef* lastToken(void) const { return lastToken_; }

    void setTokenLine(const pSourceRef* t) {
        tokenLineInfo_[t] = currentLineNum_;
    }

    void finishParse(void) {
        currentLineNum_ = 0;
        lastToken_ = NULL;
        tokenLineInfo_.clear();
    }

    const pSourceModule* getOwner(void) const {
        return owner_;
    }

    // PARSE ERROR HANDLER
    void parseError(pSourceRef* r, const pSourceRange& range);
    void parseError(pStringRef msg, const pSourceRange& range);

};

} } // namespace

// placement allocators for constructing AST nodes. this is modeled after clang
inline void *operator new(size_t bytes, corvus::AST::pParseContext& C, size_t align=8) throw () {
  return C.allocate(bytes, align);
}

inline void operator delete(void *p, corvus::AST::pParseContext& C) throw () {
  C.deallocate(p);
}

inline void *operator new[](size_t bytes, corvus::AST::pParseContext& C, size_t align = 8) throw () {
  return C.allocate(bytes, align);
}

inline void operator delete[](void *p, corvus::AST::pParseContext& C) throw () {
  C.deallocate(p);
}

#endif /* COR_PPARSECONTEXT_H_ */
