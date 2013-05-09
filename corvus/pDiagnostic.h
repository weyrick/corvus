/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PDIAGNOSTIC_H_
#define COR_PDIAGNOSTIC_H_

#include "corvus/pTypes.h"

#include <string>

namespace corvus {

class pDiagnostic {

private:

    pUInt startLineNum_;
    pUInt endLineNum_;
    pUInt startCol_;
    pUInt endCol_;

    std::string msg_;

public:    

    pDiagnostic(pUInt sl, pUInt sc, pUInt el, pUInt ec, pStringRef msg) :
        startLineNum_(sl),
        endLineNum_(el),
        startCol_(sc),
        endCol_(ec),
        msg_(msg.str()) { }

    pUInt startLineNum(void) const { return startLineNum_; }
    pUInt endLineNum(void) const { return endLineNum_; }
    pUInt startCol(void) const { return startCol_; }
    pUInt endCol(void) const { return endCol_; }

    pStringRef msg(void) const { return msg_; }

};

} // namespace

#endif
