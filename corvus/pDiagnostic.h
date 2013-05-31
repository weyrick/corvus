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

public:    

    pDiagnostic(const pSourceLoc &loc, pStringRef msg) :
        loc_(loc),
        msg_(msg.str()) { }

    const pSourceLoc& location(void) const { return loc_; }

    pStringRef msg(void) const { return msg_; }

};

} // namespace

#endif
