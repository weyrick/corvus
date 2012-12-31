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

namespace corvus {

class pParseError : public std::runtime_error {

    std::string msg_;

public:
    pParseError(const std::string& msg):
        std::runtime_error(""),
        msg_(msg) { }

    pParseError(const std::wstring& msg):
        std::runtime_error(""),
        msg_(msg.begin(), msg.end()) { }

    ~pParseError(void) throw() { }

    const char* what() const throw() {
        return msg_.c_str();
    } 

};

} // namespace

#endif /* COR_PPARSEERROR_H_ */
