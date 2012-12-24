/* ***** BEGIN LICENSE BLOCK *****
;; corvus analyzer
;;
;; Copyright (c) 2008-2010 Shannon Weyrick <weyrick@mozek.us>
;;
;; This program is free software; you can redistribute it and/or
;; modify it under the terms of the GNU General Public License
;; as published by the Free Software Foundation; either version 2
;; of the License, or (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, write to the Free Software
;; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
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
    pSourceModule(const std::string& file);
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
