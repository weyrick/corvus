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

#include "corvus/pAST.h"
#include "corvus/pParseContext.h"

#include <vector>

namespace corvus {

class pSourceFile;
class pSourceModule;
class pDiagnostic;
class pSourceManager;

namespace AST {
    class stmt;
    class pBaseVisitor;
}

class pSourceModule {
public:
    typedef std::vector<pDiagnostic *> DiagListType;

private:
    const pSourceFile* source_;
    AST::block* ast_;
    AST::pParseContext context_;
    bool parsed_;
    std::vector<pDiagnostic *> diagList_;
    pSourceManager *sourceMgr_;

public:
    pSourceModule(pSourceManager *mgr, pStringRef file);
    ~pSourceModule();

    void parse(bool debug);

    // INSPECTION
    const pSourceFile* source() const { return source_; }
    const std::string& fileName() const;

    const AST::pParseContext& context(void) const { return context_; }
    AST::pParseContext& context(void) { return context_; }

    void dumpContextStats(void) { context_.allocator().PrintStats(); }

    // AST TRAVERSAL
    AST::block* getAST() { return ast_; }
    const AST::block* getAST() const { return ast_; }
    void setAST(const AST::statementList* list);
    void applyVisitor(AST::pBaseVisitor* v);

    // DIAGNOSTICS
    void addDiagnostic(pDiagnostic* d);
    DiagListType& getDiagnostics(void);

};

} // namespace

#endif /* COR_PSOURCEMODULE_H_ */
