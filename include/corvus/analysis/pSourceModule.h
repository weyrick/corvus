/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2010 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PSOURCEMODULE_H_
#define COR_PSOURCEMODULE_H_

#include "corvus/analysis/pAST.h"
#include "corvus/analysis/pParseContext.h"

namespace corvus {

class pSourceFile;

namespace AST {
    class stmt;
    class pBaseVisitor;
    class pBaseTransformer;
}

class pSourceModule {

private:
    const pSourceFile* source_;
    AST::block* ast_;
    AST::pParseContext context_;
    bool parsed_;

public:
    pSourceModule(pStringRef file);
    ~pSourceModule();

    void parse(bool debug);

    // INSPECTION
    const pSourceFile* source() const { return source_; }
    const std::string& fileName() const;

    const AST::pParseContext& context(void) const { return context_; }
    AST::pParseContext& context(void) { return context_; }

    void dumpContextStats(void) { context_.allocator().PrintStats(); }

    // AST TRAVERSAL AND TRANSFORM
    AST::block* getAST() { return ast_; }
    const AST::block* getAST() const { return ast_; }
    void setAST(const AST::statementList* list);
    void applyVisitor(AST::pBaseVisitor* v);
    void applyTransform(AST::pBaseTransformer* t);

};

} // namespace

#endif /* COR_PSOURCEMODULE_H_ */
