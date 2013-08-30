/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PNSVISITOR_H_
#define COR_PNSVISITOR_H_

#include "corvus/pAST.h"
#include "corvus/pBaseVisitor.h"
#include "corvus/pModel.h"

#include <vector>

#define RESOLVE_FQN(sym) \
    ( (ns_use_list_.find(sym) != ns_use_list_.end()) ? ns_use_list_[sym] : sym)

namespace corvus {

namespace AST { namespace Pass {

class pNSVisitor: public pBaseVisitor {

protected:

    pModel::oid ns_id_;
    std::map<std::string, std::string> ns_use_list_;

public:

    pNSVisitor(const char* name, const char* desc): pBaseVisitor(name,desc) { }

    void pre_run(void);

    void visit_pre_namespaceDecl(namespaceDecl* n);
    void visit_post_namespaceDecl(namespaceDecl* n);

    void visit_post_useIdent(useIdent* n);

};

} } } // namespace

#endif
