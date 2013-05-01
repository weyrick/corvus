/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PMODELSCOPE_H_
#define COR_PMODELSCOPE_H_

namespace corvus {

class pNamespace {
public:
    enum kind { GLOBAL, CLASS, FUNCTION };

private:
    kind nsKind_;
    pNamespace *parent_;
    std::string name_; // php namespace name, i.e. \ or \foo

    // map string(symbol)->pDecl

public:

    pNamespace(): parent_(0), nsKind_(GLOBAL) { }
    pNamespace(pNamespace *parent, kind k): parent_(parent), nsKind_(k) { }

    bool isGlobal() { return nsKind_ == GLOBAL; }
    bool isClass() { return nsKind_ == CLASS; }
    bool isFunction() { return nsKind_ == FUNCTION; }

};

} // namespace

#endif
