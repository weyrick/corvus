/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/pSourceModule.h"
#include "corvus/analysis/pSourceFile.h"

#include "corvus/analysis/pBaseVisitor.h"
#include "corvus/analysis/pBaseTransformer.h"
#include "corvus/analysis/pParser.h"

namespace corvus {

pSourceModule::pSourceModule(pStringRef file):
    source_(new pSourceFile(file)),
    ast_(NULL),
    context_(this)
{


}

void pSourceModule::parse(bool debug=false) {

    parser::parseSourceFile(this,debug);

}

pSourceModule::~pSourceModule() {
    // cleanup AST
    if (ast_)
        ast_->destroy(context_);
    delete source_;
}

const std::string &pSourceModule::fileName() const {
    return source_->fileName();
}

void pSourceModule::setAST(const AST::statementList* list) {
    if (ast_)
        ast_->destroy(context_);
    ast_ = new (context_) AST::block(context_, list);
}

void pSourceModule::applyVisitor(AST::pBaseVisitor* v) {
    assert(ast_);
    v->visit(ast_);
}

void pSourceModule::applyTransform(AST::pBaseTransformer* t) {
    assert(ast_);
    ast_ = cast<AST::block>(t->transform(ast_));
}

} // namespace
