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

#include "pTypes.h"
#include "pSourceLoc.h"

#include <string>

namespace corvus {

class pDiagnostic {

private:

    pSourceLoc loc_;
    std::string msg_;
    int seq_; // so we can maintain order of diags as added by passes when
              // on the same line/col

public:    

    pDiagnostic(const pSourceLoc &loc, pStringRef msg, int seq=0) :
        loc_(loc),
        msg_(msg.str()),
        seq_(seq)
    { }

    void setSeq(int s) { seq_ = s; }

    const pSourceLoc& location(void) const { return loc_; }

    pStringRef msg(void) const { return msg_; }

    int seq(void) const { return seq_; }

};

} // namespace

#endif
