/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PPARSEERROR_H_
#define COR_PPARSEERROR_H_

#include <string>
#include <stdexcept>
#include "corvus/pSourceLoc.h"

namespace corvus {

class pParseError : public std::runtime_error {

    pSourceLoc loc_;
    std::string msg_;

public:
    pParseError(const std::string& msg, const pSourceLoc& loc):
        std::runtime_error(""),
        loc_(loc),
        msg_(msg) { }

    pParseError(const std::wstring& msg, const pSourceLoc& loc):
        std::runtime_error(""),
        loc_(loc),
        msg_(msg.begin(), msg.end()) { }

    ~pParseError(void) throw() { }

    pSourceLoc loc(void) const { return loc_; }

    const char* what() const throw() {
        return msg_.c_str();
    }

};

} // namespace

#endif /* COR_PPARSEERROR_H_ */
