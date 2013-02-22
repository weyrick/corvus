/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PLEXER_H_
#define COR_PLEXER_H_

#include "corvus/pTypes.h"

// NOTE: these are generated during build, and so live
// in the build directory (not source directory)
#include "corvus_grammar.h"
#include "corvus_lang_lexer.h"

namespace corvus {

class pSourceFile;

namespace lexer {

typedef lexertl::recursive_match_results<pSourceCharIterator> rmatch;

class pLexer {

private:
    const pSourceFile* source_;

    pSourceCharIterator sourceBegin_;
    pSourceCharIterator sourceEnd_;

public:

    pLexer(const pSourceFile* s);

    const pSourceCharIterator sourceBegin(void) const;
    const pSourceCharIterator sourceEnd(void) const;

    void dumpTokens(void);
    const char* getTokenDescription(const std::size_t t) const;

};

} } // namespace

#endif /* COR_PLEXER_H_ */
