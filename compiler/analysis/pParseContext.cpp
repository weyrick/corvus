/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2010 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/


#include "corvus/analysis/pParseContext.h"
#include "corvus/analysis/pSourceModule.h"
#include "corvus/analysis/pSourceFile.h"
#include "corvus/analysis/pParseError.h"

#include <iostream>
#include <sstream>

namespace corvus {

namespace AST {

pColRange pParseContext::getColPair(pSourceRange* r) {
    // find the closest newline to the left of r->begin, without underrunning
    // the buffer
    pSourceCharIterator bufBegin = owner_->source()->contents()->getBufferStart();
    pSourceCharIterator i = r->begin();
    while (i != bufBegin && *i != '\n')
        i--;
    return pColRange(r->begin()-i, r->end()-i);
}

void pParseContext::parseError(pStringRef msg) {
    std::stringstream errorMsg;

    errorMsg  << std::string(msg)
              << " in ";
    errorMsg  << owner_->source()->fileName();
    errorMsg  << " on line "
              << currentLineNum_
              <<  std::endl;

    throw pParseError(errorMsg.str());
}

void pParseContext::parseError(pSourceRange* r) {

    // show the line the error occured on
    // if it was a lex error, we show 1 character. if it was a parse error,
    // we show up the length of the problem token
    pUInt probsize;
    std::string problem;
    std::stringstream errorMsg;

    assert(lastToken_);

    // this only happens when there's a parse error due to a non matching production where
    // lastToken_ that caused the no-match is the last token in the script
    // in this case, r is null and is called from lemon. we call error again with lastToken
    // as the parseError
    bool endOfSource(false);
    if (lastToken_->end() == owner_->source()->contents()->getBufferEnd())
        endOfSource = true;

    if (r) {
        probsize = (*r).end()-(*r).begin();
        problem.append((*r).begin(), (*r).end());
    }
    else {
        if (endOfSource) {
            probsize = 0;
            problem.append(lastToken_->begin(), lastToken_->end());
        }
        else {
            probsize = 1;
            problem.append(lastToken_->end(), lastToken_->end()+1);
        }
    }

    pSourceCharIterator eLineStart(lastNewline_+1);
    pSourceCharIterator eLineStop(lastToken_->end()+probsize);

    // try to take eLineStop to next new line
    if (eLineStop < owner_->source()->contents()->getBufferEnd()) {
        while ( (*eLineStop != '\n') && (eLineStop < (owner_->source()->contents()->getBufferEnd())) ) {
            ++eLineStop;
        }
    }

    std::string errorLine;
    if (eLineStop > eLineStart)
        errorLine.append(eLineStart, eLineStop);

    // message
    errorMsg  << owner_->source()->fileName()
              << ":" << currentLineNum_
              << ": parse error: unexpected '"
              << problem
              << "'"
              <<  std::endl;

    // error line with arrow
    if (!errorLine.empty() && !endOfSource) {
        // convert tabs to spaces so arrow lines up
        for (unsigned i=0; i != errorLine.length(); i++) {
            if (errorLine[i] == '\t')
                errorLine[i] = ' ';
        }
        errorMsg << errorLine << std::endl;
        errorMsg << std::string((lastToken_->end()+1)-(lastNewline_+1)-1,' ') << "^" << std::endl;
    }

    throw pParseError(errorMsg.str());

}


} } // namespace

