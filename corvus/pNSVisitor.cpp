/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/pNSVisitor.h"

#include <string>

namespace corvus { namespace AST { namespace Pass {


void pNSVisitor::pre_run(void) {
    ns_id_ = model_->getRootNamespaceOID();
    assert(ns_id_ != pModel::NULLID && "namespace not found");
}

void pNSVisitor::visit_pre_namespaceDecl(namespaceDecl* n) {

    ns_id_ = model_->getNamespaceOID(n->name(), true);

}

void pNSVisitor::visit_post_namespaceDecl(namespaceDecl* n) {

    // we only lose the namespace if this one had a body, i.e. block
    if (n->body()) {
        ns_id_ = model_->getRootNamespaceOID();
        ns_use_list_.clear();
    }

}

void pNSVisitor::visit_post_useIdent(useIdent* n) {

    std::string alias;

    // use \foo\myclass
    //     ~~~~~~~~~~~~ = nsname, blank alias

    // if we don't have an alias, we want to extract the symbolname
    // which is myclass in this case

    // use \foo\myclass as bar
    //     ~~~~~~~~~~~~    ~~~ = alias

    // then anytime we see symbol 'alias', we instead use 'nsname'

    alias = n->alias();
    if (alias.empty()) {
        alias = n->nsname();
        if (alias.find('\\') != std::string::npos) {
            alias = alias.substr(alias.find_last_of('\\')+1);
        }
    }

    //std::cout << "use: " << alias << " => " << n->nsname().str() << "\n";
    ns_use_list_[alias] = n->nsname();

}


} } } // namespace

