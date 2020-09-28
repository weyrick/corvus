/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/pParser.h"

#include "corvus/pLexer.h"
#include "corvus/pSourceModule.h"
#include "corvus/pParseContext.h"

#include <boost/pool/object_pool.hpp>

#include <iostream>
#include <stdio.h>
#include <assert.h>

/* generated corvus_grammar parser interface */
void* corvusParseAlloc(void *(*)(size_t));
void  corvusParse(void *, int, corvus::pSourceRef*, corvus::pSourceModule*);
void  corvusParseFree(void *, void (*)(void*));
void  corvusParseTrace(FILE *, char *);

namespace corvus { namespace parser {

void countNewlines(AST::pParseContext& context, lexer::rmatch& match, pSourceCharIterator& lastNL) {
    pUInt nlCnt(0);
    //std::cout << "countNewLines [" << match.str() << "]\n";
    for (pSourceCharIterator i = match.start; i != match.end; ++i) {
        if (*i == '\n') {
            nlCnt++;
            lastNL = i;
        }
    }
    if (nlCnt) {
        //std::cout << "inc by: " << nlCnt << std::endl;
        context.incLineNum(nlCnt);
        context.setLastNewline(lastNL);
    }
}

// find the heredoc id, or else parse error looking for it
// returns pointers to the matching end token (HEREDOC_ID)
std::pair<pSourceCharIterator,pSourceCharIterator> find_heredoc_id(const std::string& HEREDOC_ID,
                     const lexer::pLexer& lexer,
                     lexer::rmatch& match,
                     pSourceModule* pMod) {
    AST::pParseContext& context = pMod->context();
    match.end--;
    look_for_id:
    if (lexer.sourceEnd() - match.end < HEREDOC_ID.length()) {
        // the remaining source text is shorter than the heredocid length,
        // which means we're never going to match it
        context.parseError("dangling HEREDOC", pSourceRange());
    }
    pSourceCharIterator ms = match.end;
    pSourceCharIterator me = match.end+HEREDOC_ID.length();
    std::string maybeID(ms, me);
    if (maybeID != HEREDOC_ID) {
        while ((*match.end != '\n') && (match.end != lexer.sourceEnd())) {
            match.end++;
        }
        match.end++; // skip newline
        goto look_for_id;
    }

    /*
 look_for_id:
    while ((*match.end != '\n') && (match.end != lexer.sourceEnd())) {
        match.end++;
    }
    if (lexer.sourceEnd() - match.end < HEREDOC_ID.length()) {
        // the remaining source text is shorter than the heredocid length,
        // which means we're never going to match it
        context.parseError("dangling HEREDOC");
        // XXX does not reach this, parseError throws
    }
    match.end++; // skip newline
    pSourceCharIterator ms = match.end;
    pSourceCharIterator me = match.end+HEREDOC_ID.length();
    std::string maybeID(ms, me);
    if (maybeID != HEREDOC_ID)
        goto look_for_id;*/
    return std::pair<pSourceCharIterator, pSourceCharIterator>(ms, me);
}

void parseSourceFile(pSourceModule* pMod, bool debug=false) {

    boost::object_pool<pSourceRef> tokenPool;
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
    context.setLastToken(tokenPool.construct(pSourceRef(lexer.sourceBegin(), 0)));
    context.setLastNewline(lexer.sourceBegin());

    pSourceRef* curRange;
    pSourceCharIterator lastNL;

    bool inlineHtml = false;
    std::string HEREDOC_ID;

    pSourceCharIterator sourceEnd(lexer.sourceEnd());
    lexer::rmatch match(lexer.sourceBegin(), lexer.sourceEnd());

    do {

        corvus_nextLangToken(match);

        //std::cout << "match: [" << match.str() << "]\n";

        // always make a range unless the scanner didn't match, in which case
        // we handle separately below
        if (match.id != match.npos()) {
            curRange = tokenPool.construct(pSourceRef(match.start, match.end-match.start));
            context.setTokenLine(curRange);
        }

        switch (match.id) {
            case 0:
            {
                // end of input (success)
                break;
            }
            case T_CLOSE_TAG:
            {
                // we swallow one newline if it follows
                //if ((match.end != sourceEnd) && (*match.end == '\n'))
                //    ++tokEnd;
                break;
            }
            case T_OPEN_TAG:
            {
                // state change (no parse), but count newlines from OPEN tag
                countNewlines(context, match, lastNL);
                break;
            }
            case std::numeric_limits<int>::max(): // npos
            {
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
                    curRange = tokenPool.construct(pSourceRef(match.start, match.end-match.start));
                    context.setTokenLine(curRange);
                    countNewlines(context, match, lastNL);
                }
                // if state is HEREDOC, collect heredoc string, looking for heredoc id
                else if (match.state == 3) {
                    // assert we have a heredoc ID
                    assert(HEREDOC_ID.length() && "no heredoc id");
                    std::pair<pSourceCharIterator,pSourceCharIterator> idr = find_heredoc_id(HEREDOC_ID, lexer, match, pMod);
                    countNewlines(context, match, lastNL);
                    curRange = tokenPool.construct(pSourceRef(match.start, match.end-match.start));
                    context.setTokenLine(curRange);
                    corvusParse(pParser, T_HEREDOC_STRING, curRange, pMod);
                    match.start = idr.first;
                    match.end = idr.second;
                    curRange = tokenPool.construct(pSourceRef(match.start, match.end-match.start));
                    context.setTokenLine(curRange);
                    corvusParse(pParser, T_HEREDOC_END, curRange, pMod);
                    match.state = 1;
                    HEREDOC_ID.clear();
                }
                else {
                    // unmatched token: error
                    pMod->context().parseError(curRange, pSourceRange());
                }
                break;
            }
            case T_HEREDOC_START:
            {
                // save the heredoc id so we can match the end
                pSourceCharIterator ms = match.start;
                while (*ms == '<' ||
                       *ms == ' ' ||
                       *ms == '\t' ||
                       *ms == '\'' ||
                       *ms == '"'
                       )
                    ms++;
                if (*(match.end-2) == '"' || *(match.end-2) == '\'')
                    HEREDOC_ID.assign(ms, match.end-2);
                else
                    HEREDOC_ID.assign(ms, match.end-1);
                countNewlines(context, match, lastNL);
                corvusParse(pParser, T_HEREDOC_START, curRange, pMod);
                break;
            }
            case T_WHITESPACE:
            case T_INLINE_HTML:
            case T_DOC_COMMENT:
            case T_MULTILINE_COMMENT:
            case T_SINGLELINE_COMMENT:
            {
                // handle newlines
                countNewlines(context, match, lastNL);
                break;
            }
            default:
            {
                // parse
                corvusParse(pParser, match.id, curRange, pMod);
                break;
            }
        }

        // next token
        context.setLastToken(curRange);

    }
    while (match.id != 0);

    // finish parse
    corvusParse(pParser, 0, 0, pMod); // note, this may generate a parse error still
    context.finishParse(); // so don't finish until here
    corvusParseFree(pParser, free);

}


} } // namespace
