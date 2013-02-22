/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

    bool inlineHtml = false;

    pSourceCharIterator sourceEnd(lexer.sourceEnd());
    lexer::rmatch match(lexer.sourceBegin(), lexer.sourceEnd());

    do {

        corvus_nextLangToken(match);

        // always make a range unless the scanner didn't match, in which case
        // we handle separately below
        if (match.id != match.npos()) {
            curRange = tokenPool.construct(pSourceRange(match.start, match.end-match.start));
            context.setTokenLine(curRange);
        }

        switch (match.id) {
            case 0:
                // end of input (success)
                break;
            case T_CLOSE_TAG:
                // we swallow one newline if it follows
                //if ((match.end != sourceEnd) && (*match.end == '\n'))
                //    ++tokEnd;
                break;
            case T_OPEN_TAG:
                // state change (no parse), but count newlines from OPEN tag
                goto handleNewlines;
            case ~0: // npos
                // if state is HTML, collect characters for INLINE HTML token
                if (match.state == 0) {
                    // we go until a single < is found, or end of input
                    // this potentially breaks up inline htmls
                    // at tags that don't turn out to be php open tags,
                    // but that way we let the lexer handle the matching
                    // and limit the special handler code here                    
                    while ((*match.end != '<') && (match.end != sourceEnd)) {
                        match.end++;
                    }
                    inlineHtml = true;
                    curRange = tokenPool.construct(pSourceRange(match.start, match.end-match.start));
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
                for (pSourceCharIterator i = match.start; i != match.end; ++i) {
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
                /*
                if (inlineHtml) {
                    corvusParse(pParser, match.id, curRange, pMod);
                    inlineHtml = false;
                }
                */
                break;
            default:
                // parse
                corvusParse(pParser, match.id, curRange, pMod);
                break;
        }

        // next token
        nlCnt = 0;
        context.setLastToken(curRange);

    }
    while (match.id != 0);

    // finish parse
    corvusParse(pParser, 0, 0, pMod); // note, this may generate a parse error still
    context.finishParse(); // so don't finish until here
    corvusParseFree(pParser, free);

}


} } // namespace
