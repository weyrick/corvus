/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2010 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

// NOTE: this is generated by lemon during build, and so lives
// in the build directory (not source directory)
#include "corvus_grammar.h"

#include "corvus/analysis/lexer/debug.hpp"

#include "corvus/analysis/lexer/generator.hpp"
#include "corvus/analysis/lexer/generate_cpp.hpp"
#include "corvus/analysis/lexer/input.hpp"
#include "corvus/analysis/lexer/state_machine.hpp"

#include <iostream>
#include <fstream>

#define IDCHARS "[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*"

int main(void) {

    // language lexer
    boost::lexer::rules langRules_((boost::lexer::regex_flags)(boost::lexer::icase/* | boost::lexer::dot_not_newline*/));
    boost::lexer::state_machine langState_;

    // double quote lexer
    boost::lexer::rules dqRules_;
    boost::lexer::state_machine dqState_;

    // double quote rules
    dqRules_.add_macro ("IDCHARS", IDCHARS);
    dqRules_.add("INITIAL", "\\\\n", T_DQ_NEWLINE, ".");
    dqRules_.add("INITIAL", "\\\"", T_DQ_DQ, ".");
    dqRules_.add("INITIAL", "\\\\\\\"", T_DQ_ESCAPE, ".");
    dqRules_.add("INITIAL", "\\${IDCHARS}", T_DQ_VARIABLE, ".");
    boost::lexer::generator::build (dqRules_, dqState_);
    boost::lexer::generator::minimise(dqState_);

    std::ofstream outFile("corvus_dq_lexer.h", std::ios::out|std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "unable to open output file corvus_dq_lexer.h" << std::endl;
        exit(-1);
    }
    boost::lexer::generate_cpp (dqState_, outFile, false, true, true, "corvus_nextDQToken");
    outFile.close();

    // language rules
    langRules_.add_state("PHP");
    langRules_.add_state("OBJPROP");

    langRules_.add_macro ("TRI", "\\?\\?");
    langRules_.add_macro ("BACKSLASH", "(\\\\|{TRI}\\/)");
    langRules_.add_macro ("DIGIT", "[0-9]");
    langRules_.add_macro ("OCTALDIGIT", "[0-7]");
    langRules_.add_macro ("HEXDIGIT", "[0-9a-fA-F]");
    langRules_.add_macro ("IDCHARS", IDCHARS);
    langRules_.add_macro ("ANY", "[^]");
    langRules_.add_macro ("NEWLINE", "(\\r|\\n|\\r\\n)");

    langRules_.add("INITIAL", "(<\\?|<\\?PHP)([ \\t]|{NEWLINE})", T_OPEN_TAG, "PHP"); // go to PHP state

    langRules_.add("PHP", "\\?>", T_CLOSE_TAG, "INITIAL"); // go to HTML state

    langRules_.add("PHP", "\\`", T_TICK, ".");
    langRules_.add("PHP", "\\~", T_TILDE, ".");
    langRules_.add("PHP", "\\(", T_LEFTPAREN, ".");
    langRules_.add("PHP", "\\)", T_RIGHTPAREN, ".");
    langRules_.add("PHP", "\\{", T_LEFTCURLY, ".");
    langRules_.add("PHP", "\\}", T_RIGHTCURLY, ".");
    langRules_.add("PHP", "\\[", T_LEFTSQUARE, ".");
    langRules_.add("PHP", "\\]", T_RIGHTSQUARE, ".");
    langRules_.add("PHP", "\\>", T_GREATER_THAN, ".");
    langRules_.add("PHP", "\\<", T_LESS_THAN, ".");
    langRules_.add("PHP", "\\=", T_ASSIGN, ".");
    langRules_.add("PHP", "\\-", T_MINUS, ".");
    langRules_.add("PHP", "\\+", T_PLUS, ".");
    langRules_.add("PHP", "\\/", T_DIV, ".");
    langRules_.add("PHP", "\\%", T_MOD, ".");
    langRules_.add("PHP", "\\*", T_MULT, ".");
    langRules_.add("PHP", "\\;", T_SEMI, ".");
    langRules_.add("PHP", "\\,", T_COMMA, ".");
    langRules_.add("PHP", "\\|", T_PIPE, ".");
    langRules_.add("PHP", "\\.", T_DOT, ".");
    langRules_.add("PHP", "\\^", T_CARET, ".");
    langRules_.add("PHP", "\\&", T_AND, ".");
    langRules_.add("PHP", "\\@", T_AT, ".");
    langRules_.add("PHP", "\\?", T_QUESTION, ".");
    langRules_.add("PHP", "\\:", T_COLON, ".");
    langRules_.add("PHP", "\\$", T_DOLLAR, ".");
    langRules_.add("PHP", "\\\\", T_NS_SEPARATOR, ".");
    langRules_.add("PHP", ">=", T_GREATER_OR_EQUAL, ".");
    langRules_.add("PHP", "<=", T_LESS_OR_EQUAL, ".");
    langRules_.add("PHP", "<<", T_SL, ".");
    langRules_.add("PHP", ">>", T_SR, ".");
    langRules_.add("PHP", "<<=", T_SL_EQUAL, ".");
    langRules_.add("PHP", ">>=", T_SR_EQUAL, ".");
    langRules_.add("PHP", "::", T_DBL_COLON, ".");
    langRules_.add("PHP", "\\+\\+", T_INC, ".");
    langRules_.add("PHP", "\\-\\-", T_DEC, ".");
    langRules_.add("PHP", "\\^\\=", T_XOR_EQUAL, ".");
    langRules_.add("PHP", "\\|\\=", T_OR_EQUAL, ".");
    langRules_.add("PHP", "\\&\\=", T_AND_EQUAL, ".");
    langRules_.add("PHP", "\\%\\=", T_MOD_EQUAL, ".");
    langRules_.add("PHP", "\\.\\=", T_CONCAT_EQUAL, ".");
    langRules_.add("PHP", "\\/\\=", T_DIV_EQUAL, ".");
    langRules_.add("PHP", "\\*\\=", T_MUL_EQUAL, ".");
    langRules_.add("PHP", "\\+\\=", T_PLUS_EQUAL, ".");
    langRules_.add("PHP", "\\-\\=", T_MINUS_EQUAL, ".");
    langRules_.add("PHP", "\\!\\=", T_NOT_EQUAL, ".");
    langRules_.add("PHP", "==", T_EQUAL, ".");
    langRules_.add("PHP", "\\!==", T_NOT_IDENTICAL, ".");
    langRules_.add("PHP", "===", T_IDENTICAL, ".");
    langRules_.add("PHP", "\\&\\&", T_BOOLEAN_AND, ".");
    langRules_.add("PHP", "\\|\\|", T_BOOLEAN_OR, ".");
    langRules_.add("PHP", "\\!", T_BOOLEAN_NOT, ".");
    langRules_.add("PHP", "and", T_BOOLEAN_AND_LIT, ".");
    langRules_.add("PHP", "or", T_BOOLEAN_OR_LIT, ".");
    langRules_.add("PHP", "xor", T_BOOLEAN_XOR_LIT, ".");
    langRules_.add("PHP", "\\?>", T_CLOSE_TAG, ".");
    langRules_.add("PHP", "=>", T_ARROWKEY, ".");

    langRules_.add("PHP", "->", T_CLASSDEREF, "OBJPROP");
    langRules_.add("OBJPROP", "{IDCHARS}", T_IDENTIFIER, "PHP");
    langRules_.add("OBJPROP", "\\${IDCHARS}", T_VARIABLE, "PHP");

    langRules_.add("PHP", "<>", T_NOT_EQUAL, ".");
    langRules_.add("PHP", "true", T_TRUE, ".");
    langRules_.add("PHP", "false", T_FALSE, ".");
    langRules_.add("PHP", "null", T_NULL, ".");
    langRules_.add("PHP", "list", T_LIST, ".");
    langRules_.add("PHP", "if", T_IF, ".");
    langRules_.add("PHP", "for", T_FOR, ".");
    langRules_.add("PHP", "endfor", T_ENDFOR, ".");
    langRules_.add("PHP", "foreach", T_FOREACH, ".");
    langRules_.add("PHP", "endforeach", T_ENDFOREACH, ".");
    langRules_.add("PHP", "interface", T_INTERFACE, ".");
    langRules_.add("PHP", "as", T_AS, ".");
    langRules_.add("PHP", "do", T_DO, ".");
    langRules_.add("PHP", "exit", T_EXIT, ".");
    langRules_.add("PHP", "print", T_PRINT, ".");
    langRules_.add("PHP", "eval", T_EXIT, ".");
    langRules_.add("PHP", "public", T_PUBLIC, ".");
    langRules_.add("PHP", "private", T_PRIVATE, ".");
    langRules_.add("PHP", "protected", T_PROTECTED, ".");
    langRules_.add("PHP", "abstract", T_ABSTRACT, ".");
    langRules_.add("PHP", "final", T_FINAL, ".");
    langRules_.add("PHP", "implements", T_IMPLEMENTS, ".");
    langRules_.add("PHP", "extends", T_EXTENDS, ".");
    langRules_.add("PHP", "return", T_RETURN, ".");
    langRules_.add("PHP", "global", T_GLOBAL, ".");
    langRules_.add("PHP", "function", T_FUNCTION, ".");
    langRules_.add("PHP", "namespace", T_NAMESPACE, ".");
    langRules_.add("PHP", "isset", T_ISSET, ".");
    langRules_.add("PHP", "unset", T_UNSET, ".");
    langRules_.add("PHP", "empty", T_EMPTY, ".");
    langRules_.add("PHP", "array", T_ARRAY, ".");
    langRules_.add("PHP", "while", T_WHILE, ".");
    langRules_.add("PHP", "endwhile", T_ENDWHILE, ".");
    langRules_.add("PHP", "else", T_ELSE, ".");
    langRules_.add("PHP", "elseif", T_ELSEIF, ".");
    langRules_.add("PHP", "echo", T_ECHO, ".");
    langRules_.add("PHP", "new", T_NEW, ".");
    langRules_.add("PHP", "var", T_VAR, ".");
    langRules_.add("PHP", "switch", T_SWITCH, ".");
    langRules_.add("PHP", "endswitch", T_ENDSWITCH, ".");
    langRules_.add("PHP", "case", T_CASE, ".");
    langRules_.add("PHP", "break", T_BREAK, ".");
    langRules_.add("PHP", "continue", T_CONTINUE, ".");
    langRules_.add("PHP", "default", T_DEFAULT, ".");
    langRules_.add("PHP", "instanceof", T_INSTANCEOF, ".");
    langRules_.add("PHP", "class", T_CLASS, ".");
    langRules_.add("PHP", "clone", T_CLONE, ".");
    langRules_.add("PHP", "throw", T_THROW, ".");
    langRules_.add("PHP", "try", T_TRY, ".");
    langRules_.add("PHP", "catch", T_CATCH, ".");
    langRules_.add("PHP", "goto", T_GOTO, ".");
    langRules_.add("PHP", "const", T_CONST, ".");
    langRules_.add("PHP", "static", T_STATIC, ".");
    langRules_.add("PHP", "include", T_INCLUDE, ".");
    langRules_.add("PHP", "include_once", T_INCLUDE_ONCE, ".");
    langRules_.add("PHP", "require", T_REQUIRE, ".");
    langRules_.add("PHP", "require_once", T_REQUIRE_ONCE, ".");
    langRules_.add("PHP", "namespace", T_NAMESPACE, ".");
    langRules_.add("PHP", "__FILE__", T_MAGIC_FILE, ".");
    langRules_.add("PHP", "__LINE__", T_MAGIC_LINE, ".");
    langRules_.add("PHP", "__CLASS__", T_MAGIC_CLASS, ".");
    langRules_.add("PHP", "__METHOD__", T_MAGIC_METHOD, ".");
    langRules_.add("PHP", "__FUNCTION__", T_MAGIC_FUNCTION, ".");
    langRules_.add("PHP", "__NAMESPACE__", T_MAGIC_NS, ".");
    langRules_.add("PHP", "\\((int|integer)\\)", T_INT_CAST, ".");
    langRules_.add("PHP", "\\((real|double|float)\\)", T_FLOAT_CAST, ".");
    langRules_.add("PHP", "\\(string\\)", T_STRING_CAST, ".");
    langRules_.add("PHP", "\\(binary\\)", T_BINARY_CAST, ".");
    langRules_.add("PHP", "\\(unicode\\)", T_UNICODE_CAST, ".");
    langRules_.add("PHP", "\\(array\\)", T_ARRAY_CAST, ".");
    langRules_.add("PHP", "\\(object\\)", T_OBJECT_CAST, ".");
    langRules_.add("PHP", "\\((bool|boolean)\\)", T_BOOL_CAST, ".");
    langRules_.add("PHP", "\\(unset\\)", T_UNSET_CAST, ".");
    langRules_.add("PHP", "{IDCHARS}", T_IDENTIFIER, ".");
    langRules_.add("PHP", "\\${IDCHARS}", T_VARIABLE, ".");
    langRules_.add("PHP", "((0x|0X){HEXDIGIT}+|0{OCTALDIGIT}*|[1-9]{DIGIT}*)", T_LNUMBER, ".");
    langRules_.add("PHP", "([0-9]*[\\.][0-9]+)|([0-9]+[\\.][0-9]*)", T_DNUMBER, ".");
    langRules_.add("PHP", "([ \\t]|{NEWLINE})+", T_WHITESPACE, ".");
    langRules_.add("PHP", "\\/\\*\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/", T_DOC_COMMENT, ".");
    langRules_.add("PHP", "\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/", T_MULTILINE_COMMENT, ".");
    langRules_.add("PHP", "(\\/\\/|#).*$", T_SINGLELINE_COMMENT, ".");
    langRules_.add_macro ("ESCAPESEQ", "{BACKSLASH}([abfnrtv?'\"]|{BACKSLASH}|x{HEXDIGIT}+|{OCTALDIGIT}{1,3})");
    langRules_.add("PHP", "\\\"({ESCAPESEQ}|[^\"])*\\\"", T_DQ_STRING, ".");
    langRules_.add_macro ("ESCAPESEQ2", "{BACKSLASH}.");
    langRules_.add("PHP", "'({ESCAPESEQ2}|[^'\\\\])*'", T_SQ_STRING, ".");

    boost::lexer::generator::build (langRules_, langState_);
    boost::lexer::generator::minimise(langState_);

    std::ofstream outFile2("corvus_lang_lexer.h", std::ios::out|std::ios::trunc);
    if (!outFile2.is_open()) {
        std::cerr << "unable to open output file corvus_lang_lexer.h" << std::endl;
        exit(-1);
    }
    boost::lexer::generate_cpp (langState_, outFile2, false, true, true, "corvus_nextLangToken");
    outFile2.close();

}

