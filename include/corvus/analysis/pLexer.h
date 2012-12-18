/* ***** BEGIN LICENSE BLOCK *****
;; Roadsend PHP Compiler
;;
;; Copyright (c) 2008 Shannon Weyrick <weyrick@mozek.us>
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

#ifndef RPHP_PLEXER_H_
#define RPHP_PLEXER_H_

#include "corvus/pSourceTypes.h"
#include "corvus/analysis/lexer/consts.hpp"

// NOTE: these are generated during build, and so live
// in the build directory (not source directory)
#include "corvus_grammar.h"
#include "corvus_lang_lexer.h"

namespace corvus {

class pSourceFile;

namespace lexer {

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

#endif /* RPHP_PLEXER_H_ */
