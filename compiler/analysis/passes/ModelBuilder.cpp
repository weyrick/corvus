/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/passes/ModelBuilder.h"

#include <sqlite3.h>
#include <sstream>

namespace corvus { namespace AST { namespace Pass {


void ModelBuilder::pre_run(void) {

    scope_.push_back(MODULE);
    std::cout << "modelling " << module_->fileName() << "\n";

    // create tables if not exist
    const char *CREATE = "CREATE TABLE IF NOT EXISTS sourceModules (" \
                         "realpath TEXT UNIQUE," \
                         "hash TEXT" \
                         ")";

    char *errMsg;
    int rc = sqlite3_exec(db, CREATE, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << errMsg << "\n";
        exit(1);
    }

    std::stringstream ins;
    ins << "INSERT INTO sourceModules VALUES ('" << module_->fileName() << "', '')";
    rc = sqlite3_exec(db, ins.str().c_str(), NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << errMsg << "\n";
        exit(1);
    }

}

void ModelBuilder::post_run(void) {

}

void ModelBuilder::visit_pre_namespaceDecl(namespaceDecl* n) {

    std::cout << "now in namespace: " << n->name().str() << "\n";
    namespace_ = n->name();

}


void ModelBuilder::visit_pre_classDecl(classDecl* n) {

    std::cout << "class: " << n->name().str() << ", moving to class scope\n";
    scope_.push_back(CLASS);

}

void ModelBuilder::visit_post_classDecl(classDecl* n) {

    std::cout << "class: " << n->name().str() << ", leaving class scope\n";
    scope_.pop_back();

}

void ModelBuilder::visit_pre_signature(signature* n) {


    std::cout << "function: " << n->name().str() << ", moving to function scope\n";
    scope_.push_back(FUNCTION);

    for (int i = 0; i < n->numParams(); i++) {
        do_decl(n->getParam(i)->name());
    }

}

void ModelBuilder::visit_post_signature(signature* n) {


    std::cout << "function: " << n->name().str() << ", leaving function scope\n";
    scope_.pop_back();

}

void ModelBuilder::visit_pre_block(block* n) {


    std::cout << "moving to block scope\n";
    scope_.push_back(BLOCK);

}

void ModelBuilder::visit_post_block(block* n) {


    std::cout << "leaving block scope\n";
    scope_.pop_back();

}

void ModelBuilder::visit_pre_assignment(assignment* n) {


    expr* lval = n->lVal();
    if (var *i = llvm::dyn_cast<var>(lval)) {
        do_decl(i->name());
    }

    expr* rval = n->rVal();
    if (var *i = llvm::dyn_cast<var>(rval)) {
        do_use(i->name());
    }

}

void ModelBuilder::do_decl(const std::string& name) {

    std::string fqs(name);
    kind cur_scope = scope_.back();
    switch (cur_scope) {
        case MODULE:
            std::cout << "in scope MODULE ";
            break;
        case CLASS:
            std::cout << "in scope CLASS ";
            fqs = namespace_ + "\\" + name;
            break;
        case FUNCTION:
            std::cout << "in scope FUNCTION ";
            fqs = namespace_ + "\\" + name;
            break;
        case BLOCK:
            std::cout << "in scope BLOCK ";
            break;
    }
    std::cout << "DECL: " << fqs << "\n";

}

void ModelBuilder::do_use(const std::string& name) {

    kind cur_scope = scope_.back();
    switch (cur_scope) {
        case MODULE:
            std::cout << "in scope MODULE ";
            break;
        case CLASS:
            std::cout << "in scope CLASS ";
            break;
        case FUNCTION:
            std::cout << "in scope FUNCTION ";
            break;
        case BLOCK:
            std::cout << "in scope BLOCK ";
            break;
    }
    std::cout << "USE: " << name << "\n";

}

} } } // namespace

