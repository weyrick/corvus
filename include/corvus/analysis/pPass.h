/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2009 Shannon Weyrick <weyrick@mozek.us>
;; Copyright (c) 2010 Cornelius Riemenschneider <c.r1@gmx.de>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PPASS_H_
#define COR_PPASS_H_

#include <string>
#include <corvus/analysis/pSourceModule.h>

namespace corvus {

namespace AST {

class pPass {

private:
    // no copy constructor
    pPass(const pPass& p) { }

protected:
    std::string passName_;
    std::string passDesc_;

    pSourceModule* module_;

    static const char* nodeDescTable_[];
public:

    pPass(const char* n, const char* d): passName_(n), passDesc_(d) { }

    virtual ~pPass(void) { }

    void do_pre_run(pSourceModule *mod) { module_ = mod; pre_run(); module_ = NULL; }
    void do_run(pSourceModule *mod) { module_ = mod; run(); module_ = NULL; }
    void do_post_run(pSourceModule *mod) { module_ = mod; post_run(); module_ = NULL; }

    virtual void pre_run(void) { }
    virtual void run(void) = 0;
    virtual void post_run(void)  { }

    void addDiagnostic(AST::stmt*, pStringRef msg);

};


} } // namespace

#endif /* COR_PPASS_H_ */
