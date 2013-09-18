/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/pSourceModule.h"
#include "corvus/pSourceFile.h"
#include "corvus/pSourceManager.h"

#include "corvus/pBaseVisitor.h"
#include "corvus/pParser.h"
#include "corvus/pDiagnostic.h"

#include <algorithm>

namespace corvus {

pSourceModule::pSourceModule(pSourceManager *mgr, pStringRef file):
    source_(new pSourceFile(file)),
    ast_(NULL),
    context_(this),
    sourceMgr_(mgr)
{


}

void pSourceModule::parse(bool debug=false) {

    if (!ast_)
        parser::parseSourceFile(this,debug);

}

pSourceModule::~pSourceModule() {
    // cleanup AST
    if (ast_)
        ast_->destroy(context_);
    // cleanup diagnostics
    if (!diagList_.empty()) {
        for (int i = 0; i < diagList_.size(); ++i)
            delete diagList_[i];
    }
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

bool compareDiagLine(pDiagnostic* lhs, pDiagnostic* rhs) {
    if ((lhs->location().range().startLine == rhs->location().range().startLine) &&
        (lhs->location().range().startCol == rhs->location().range().startCol))
        return lhs->seq() < rhs->seq();
    if (lhs->location().range().startLine == rhs->location().range().startLine)
        return (lhs->location().range().startCol < rhs->location().range().startCol);
    else
        return (lhs->location().range().startLine < rhs->location().range().startLine);
}

pSourceModule::DiagListType &pSourceModule::getDiagnostics(void) {
    // sort
    std::sort(diagList_.begin(), diagList_.end(), compareDiagLine);
    // return
    return diagList_;
}

void pSourceModule::addDiagnostic(pDiagnostic* d) {
    // we take ownership here
    d->setSeq(diagList_.size());
    diagList_.push_back(d);
    sourceMgr_->trackDiagModule(this);
}


} // namespace
