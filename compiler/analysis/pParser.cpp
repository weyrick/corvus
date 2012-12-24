/* ***** BEGIN LICENSE BLOCK *****
;; corvus analyzer
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
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

#include "corvus/analysis/pParser.h"

#include "corvus/analysis/pLexer.h"
#include "corvus/analysis/pSourceModule.h"
#include "corvus/analysis/pParseContext.h"

#include <boost/pool/object_pool.hpp>

#include <iostream>
#include <stdio.h>

/* generated corvus_grammar parser interface */
void* corvusParseAlloc(void *(*)(size_t));
void  corvusParse(void *, int, corvus::pSourceRange*, corvus::pSourceModule*);
void  corvusParseFree(void *, void (*)(void*));
void  corvusParseTrace(FILE *, char *);

namespace corvus { namespace parser {

void parseSourceFile(pSourceModule* pMod, bool debug=false) {

    boost::object_pool<pSourceRange> tokenPool;
    lexer::pLexer lexer(pMod->source());

    void* pParser = corvusParseAlloc(malloc);

#ifndef NDEBUG
    // DEBUG
    if (debug)
        corvusParseTrace(stderr, (char*)"trace: ");
#endif

    // start at begining of source file
    AST::pParseContext& context = pMod->context();
    context.incLineNum(); // line 1
    context.setLastToken(tokenPool.construct(pSourceRange(lexer.sourceBegin(), 0)));
    context.setLastNewline(lexer.sourceBegin());

    pSourceRange* curRange;
    pSourceCharIterator lastNL;
    pUInt nlCnt(0);

    pUInt curID(0);
    std::size_t state(0), newState(0), uniqueID(0);
    pSourceCharIterator sourceEnd(lexer.sourceEnd());
    pSourceCharIterator tokStart(lexer.sourceBegin());
    pSourceCharIterator tokEnd(lexer.sourceBegin());

    while ( (curID = corvus_nextLangToken(newState, tokEnd, sourceEnd, uniqueID)) ) {

        // always make a range unless the scanner didn't match, in which case
        // we handle separately below
        if (curID != boost::lexer::npos) {
            curRange = tokenPool.construct(pSourceRange(tokStart, tokEnd-tokStart));
            context.setTokenLine(curRange);
        }

        switch (curID) {
            case 0:
                // end of input (success)
                break;
            case T_CLOSE_TAG:
                // we swallow one newline if it follows
                if ((tokEnd != sourceEnd) && (*tokEnd == '\n'))
                    ++tokEnd;
                break;
            case T_OPEN_TAG:
                // state change (no parse), but count newlines from OPEN tag
                goto handleNewlines;
            case boost::lexer::npos:
                // if state is HTML, collect characters for INLINE HTML token
                if (state == 0) {
                    // we go until a single < is found, or end of input
                    // this potentially breaks up inline htmls
                    // at tags that don't turn out to be php open tags,
                    // but that way we let the lexer handle the matching
                    // and limit the special handler code here
                    while ((*tokEnd != '<') && (tokEnd != sourceEnd)) {
                        tokEnd++;
                    }
                    curID = T_INLINE_HTML;
                    curRange = tokenPool.construct(pSourceRange(tokStart, tokEnd-tokStart));
                    context.setTokenLine(curRange);
                    goto handleNewlines;
                }
                else {
                    // unmatched token: error
                    pMod->context().parseError(NULL);
                }
                break;
            case T_WHITESPACE:
            case T_INLINE_HTML:
            case T_DOC_COMMENT:
            case T_MULTILINE_COMMENT:
            case T_SINGLELINE_COMMENT:
                // handle newlines
                handleNewlines:
                for (pSourceCharIterator i = tokStart; i != tokEnd; ++i) {
                    if (*i == '\n') {
                        nlCnt++;
                        lastNL = i;
                    }
                }
                if (nlCnt) {
                    context.incLineNum(nlCnt);
                    context.setLastNewline(lastNL);
                }
                // only actually parse T_INLINE_HTML, not whitespace
                if (curID == T_INLINE_HTML) {
                    corvusParse(pParser, curID, curRange, pMod);
                }
                break;
            default:
                // parse
                corvusParse(pParser, curID, curRange, pMod);
                break;
        }

        // next token
        nlCnt = 0;
        tokStart = tokEnd;
        state = newState;
        context.setLastToken(curRange);

    }

    // finish parse
    corvusParse(pParser, 0, 0, pMod); // note, this may generate a parse error still
    context.finishParse(); // so don't finish until here
    corvusParseFree(pParser, free);

}


} } // namespace
