/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/pLexer.h"
#include "corvus/analysis/pSourceFile.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <assert.h>

namespace corvus { namespace lexer {

pLexer::pLexer(const pSourceFile* s):
    source_(s),
    sourceBegin_(source_->contents()->getBufferStart()),
    sourceEnd_ (source_->contents()->getBufferEnd())
{



}

const pSourceCharIterator pLexer::sourceBegin(void) const {
    return sourceBegin_;
}

const pSourceCharIterator pLexer::sourceEnd(void) const {
    return sourceEnd_;
}

void pLexer::dumpTokens(void) {

    std::string tokID;
    std::stringstream val;
    std::string HEREDOC_ID;

    rmatch match(sourceBegin_, sourceEnd_);

    do {

        corvus_nextLangToken(match);
        if (match.id == 0) {
            // end of input
            break;
        }
        else if (match.id == match.npos()) {
            // if state is HTML, collect characters for INLINE HTML token
            if (match.state == 0) {
                while ((*match.end != '<') && (match.end != sourceEnd_)) {
                    match.end++;
                }
                std::cout << match.str() << " " << getTokenDescription(T_INLINE_HTML) << std::endl;
            }
            // if state is HEREDOC, collect heredoc string, looking for heredoc id
            else if (match.state == 3) {
                // assert we have a heredoc ID
                assert(HEREDOC_ID.length() && "no heredoc id");
                look_for_id:
                    while ((*match.end != '\n') && (match.end != sourceEnd_)) {
                        match.end++;
                    }
                if (sourceEnd_ - match.end < HEREDOC_ID.length()) {
                    // the remaining source text is shorter than the heredocid length,
                    // which means we're never going to match it
                    std::cout << "dangling HEREDOC" << std::endl;
                    break;
                }
                match.end++; // skip newline
                pSourceCharIterator ms = match.end;
                pSourceCharIterator me = match.end+HEREDOC_ID.length();
                std::string maybeID(ms, me);
                if (maybeID != HEREDOC_ID) {
                    goto look_for_id;
                }
                // if we get here, we matched the heredoc id
                std::cout << match.str() << " " << getTokenDescription(T_DQ_STRING) << std::endl;
                match.start = ms;
                match.end = me;
                std::cout << match.str() << " " << getTokenDescription(T_HEREDOC_END) << std::endl;
                match.state = 1;
                HEREDOC_ID.clear();
            }
            else {
                // unmatched character in PHP state
                std::cout << "breaking on unmatched: " << match.str() << std::endl;
                break;
            }
        }
        else {
            // matched
            // skip plain newlines in html state
            val.str("");
            if (match.id == T_HEREDOC_START) {
                // save the heredoc id so we can match the end
                pSourceCharIterator ms = match.start;
                while (*ms == '<' || *ms == ' ' || *ms == '\t')
                    ms++;
                HEREDOC_ID.assign(ms, match.end-1);
                std::cout << std::string(match.start, match.end-1) << " T_HEREDOC_START" << std::endl;
                continue;
            }
            if (match.id != T_WHITESPACE)
                val << match.str();
            if ((match.state == 0) && (val.str() == "\n"))
                continue;
            tokID = getTokenDescription(match.id);
            if (tokID.size() == 0)
                tokID = val.str();
            std::cout << val.str() << " " << tokID << std::endl;
        }

    }
    while (match.id != 0);


}

// note, these are designed to line up with zend token names
const char* pLexer::getTokenDescription(const std::size_t t) const {

    switch (t) {
        case T_VARIABLE:
            return "T_VARIABLE";
        case T_WHITESPACE:
            return "T_WHITESPACE";
        case T_INLINE_HTML:
            return "T_INLINE_HTML";
        case T_ECHO:
            return "T_ECHO";
        case T_ARROWKEY:
            return "T_DOUBLE_ARROW";
        case T_OPEN_TAG:
            return "T_OPEN_TAG";
        case T_CLOSE_TAG:
            return "T_CLOSE_TAG";
        case T_LNUMBER:
            return "T_LNUMBER";
        case T_DNUMBER:
            return "T_DNUMBER";
        case T_IF:
            return "T_IF";
        case T_ELSE:
            return "T_ELSE";
        case T_ELSEIF:
            return "T_ELSEIF";
        case T_WHILE:
            return "T_WHILE";
        case T_ENDWHILE:
            return "T_ENDWHILE";
        case T_NEW:
            return "T_NEW";
        case T_ARRAY:
            return "T_ARRAY";
        case T_IDENTIFIER:
            return "T_STRING";
        case T_DQ_STRING:
        case T_SQ_STRING:
            return "T_CONSTANT_ENCAPSED_STRING";
        case T_SINGLELINE_COMMENT:
        case T_MULTILINE_COMMENT:
            return "T_COMMENT";
        case T_DOC_COMMENT:
            return "T_DOC_COMMENT";
        case T_GLOBAL:
            return "T_GLOBAL";
        case T_FUNCTION:
            return "T_FUNCTION";
        case T_EMPTY:
            return "T_EMPTY";
        case T_BOOLEAN_AND:
            return "T_BOOLEAN_AND";
        case T_BOOLEAN_OR:
            return "T_BOOLEAN_OR";
        case T_EQUAL:
            return "T_IS_EQUAL";
        case T_ISSET:
            return "T_ISSET";
        case T_UNSET:
            return "T_UNSET";
        case T_VAR:
            return "T_VAR";
        case T_CLASS:
            return "T_CLASS";
        case T_CLASSDEREF:
            return "T_OBJECT_OPERATOR";
        case T_INSTANCEOF:
            return "T_INSTANCEOF";
        case T_FOREACH:
            return "T_FOREACH";
        case T_FOR:
            return "T_FOR";
        case T_ENDFOREACH:
            return "T_ENDFOREACH";
        case T_ENDFOR:
            return "T_ENDFOR";
        case T_AS:
            return "T_AS";
        case T_RETURN:
            return "T_RETURN";
        case T_LIST:
            return "T_LIST";
        case T_EXTENDS:
            return "T_EXTENDS";
        case T_PUBLIC:
            return "T_PUBLIC";
        case T_PRIVATE:
            return "T_PRIVATE";
        case T_PROTECTED:
            return "T_PROTECTED";
            /*
        case T_INCLUDE:
            return "T_INCLUDE";
        case T_INCLUDE_ONCE:
            return "T_INCLUDE_ONCE";
        case T_REQUIRE:
            return "T_REQUIRE";
        case T_REQUIRE_ONCE:
            return "T_REQUIRE_ONCE";
            */
        case T_IDENTICAL:
            return "T_IS_IDENTICAL";
        case T_NOT_IDENTICAL:
            return "T_IS_NOT_IDENTICAL";
        case T_DBL_COLON:
            return "T_DOUBLE_COLON";
        case T_INC:
            return "T_INC";
        case T_DEC:
            return "T_DEC";
        case T_EXIT:
            return "T_EXIT";
        case T_SWITCH:
            return "T_SWITCH";
        case T_ENDSWITCH:
            return "T_ENDSWITCH";
        case T_CASE:
            return "T_CASE";
        case T_BREAK:
            return "T_BREAK";
        case T_CONTINUE:
            return "T_CONTINUE";
        case T_DEFAULT:
            return "T_DEFAULT";
        case T_CLONE:
            return "T_CLONE";
        case T_TRY:
            return "T_TRY";
        case T_CATCH:
            return "T_CATCH";
        case T_THROW:
            return "T_THROW";
        case T_STATIC:
            return "T_STATIC";
        case T_CONST:
            return "T_CONST";
        case T_GREATER_OR_EQUAL:
            return "T_IS_GREATER_OR_EQUAL";
        case T_LESS_OR_EQUAL:
            return "T_IS_SMALLER_OR_EQUAL";
        case T_BOOLEAN_OR_LIT:
            return "T_LOGICAL_OR";
        case T_BOOLEAN_XOR_LIT:
            return "T_LOGICAL_XOR";
        case T_BOOLEAN_AND_LIT:
            return "T_LOGICAL_AND";
        case T_PLUS_EQUAL:
            return "T_PLUS_EQUAL";
        case T_MINUS_EQUAL:
            return "T_MINUS_EQUAL";
        case T_SR:
            return "T_SR";
        case T_SL:
            return "T_S";
        case T_SR_EQUAL:
            return "T_SR_EQUAL";
        case T_SL_EQUAL:
            return "T_SL_EQUAL";
        case T_EVAL:
            return "T_EVA";
        case T_XOR_EQUAL:
            return "T_XOR_EQUAL";
        case T_OR_EQUAL:
            return "T_OR_EQUAL";
        case T_AND_EQUAL:
            return "T_AND_EQUAL";
        case T_MOD_EQUAL:
            return "T_MOD_EQUAL";
        case T_CONCAT_EQUAL:
            return "T_CONCAT_EQUAL";
        case T_DIV_EQUAL:
            return "T_DIV_EQUAL";
        case T_MUL_EQUAL:
            return "T_MUL_EQUAL";
        case T_NAMESPACE:
            return "T_NAMESPACE";
        case T_INT_CAST:
            return "T_INT_CAST";
        case T_FLOAT_CAST:
            return "T_DOUBLE_CAST";
        case T_STRING_CAST:
            return "T_STRING_CAST";
        case T_UNICODE_CAST:
            return "T_UNICODE_CAST";
        case T_BINARY_CAST:
            return "T_BINARY_CAST";
        case T_ARRAY_CAST:
            return "T_ARRAY_CAST";
        case T_OBJECT_CAST:
            return "T_OBJECT_CAST";
        case T_UNSET_CAST:
            return "T_UNSET_CAST";
        case T_BOOL_CAST:
            return "T_BOOL_CAST";
        case T_MAGIC_FILE:
            return "T_FILE";
        case T_MAGIC_LINE:
            return "T_LINE";
        case T_MAGIC_CLASS:
            return "T_CLASS_C";
        case T_MAGIC_FUNCTION:
            return "T_FUNCTION_C";
        case T_MAGIC_METHOD:
            return "T_METHOD_C";
        case T_MAGIC_NS:
            return "T_NS_C";
        case T_PRINT:
            return "T_PRINT";
        case T_INTERFACE:
            return "T_INTERFACE";
        case T_HEREDOC_START:
            return "T_HEREDOC_START";
        case T_HEREDOC_END:
            return "T_HEREDOC_END";
        case T_USE:
            return "T_USE";


    }
    return "";

}


} } // namespace

