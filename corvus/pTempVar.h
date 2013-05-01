/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2010 Cornelius Riemenschneider <c.r1@gmx.de>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PTEMPVAR_H_
#define COR_PTEMPVAR_H_

#include "corvus/analysis/pAST.h"

#include <sstream>

namespace corvus {

class pTempVar {
    pUInt nameCount_;
    AST::pParseContext& C_;

public:
    pTempVar(AST::pParseContext& C) : nameCount_(0), C_(C) {}
    
    std::string getTempName(pStringRef name) {
        std::stringstream ss;
        ss << "." << name.str() << nameCount_++;
        return ss.str();
    }
    
    AST::var* getTempVar(pStringRef name) {
        //TODO: Add a facility for release builds which doesn't care about name for speed.
        return new(C_) AST::var(getTempName(name), C_);
    }
};

} // namespace

#endif /* COR_PTEMPVAR_H_ */
