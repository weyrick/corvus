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

#ifndef COR_PTRANSFORMER_H_
#define COR_PTRANSFORMER_H_

#include "corvus/analysis/pAST.h"
#include "corvus/analysis/pParseContext.h"
#include "corvus/analysis/pPass.h"
#include "corvus/analysis/pTransformHelper.h"

namespace corvus { namespace AST {

class pBaseTransformer: public pPass {
private:

    typedef stmt* (pBaseTransformer::*dispatchFunction)(stmt *);

    static dispatchFunction preDispatchTable_[];
    static dispatchFunction postDispatchTable_[];

protected:
    pTransformHelper h_;

public:
    pBaseTransformer(const char* name, const char* desc, pSourceModule* m): pPass(name,desc,m), h_(C_) { }
    virtual ~pBaseTransformer() { }

    // pass
    void run(void);

    // root transform
    stmt* transform(stmt*);

    // PRE
#define STMT(CLASS, PARENT) virtual PARENT * transform_pre_##CLASS(CLASS *n) { return n; }
#include "corvus/analysis/astNodes.def"

    // POST
#define STMT(CLASS, PARENT) virtual PARENT * transform_post_##CLASS(CLASS *n) { return n; }
#include "corvus/analysis/astNodes.def"


};


} } // namespace

#endif /* COR_PTRANSFORMER_H_ */
