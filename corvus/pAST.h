/* ***** BEGIN LICENSE BLOCK *****

   Copyright (c) 2008-2010 Shannon Weyrick <weyrick@mozek.us>
   Copyright (c) 2010 Cornelius Riemenschneider <c.r1@gmx.de>

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****

 The layout of this AST has taken much inspiration from clang,
 llvm, and kdevelop-pg, among others. Particularly, the base stmt
 class using the bump ptr and iterators are based on the ones in the
 AST library in clang.

 http://clang.llvm.org
 http://kdevelop.org

*/

#ifndef COR_PAST_H_
#define COR_PAST_H_

#include "corvus/pTypes.h"
#include "corvus/pParseContext.h"

#include <vector>
#include <iterator>
#include <boost/range/iterator_range.hpp>

#include <llvm/Support/Casting.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/APInt.h>

namespace corvus {

using llvm::isa;
using llvm::dyn_cast;
using llvm::cast;

class pSourceModule;

namespace AST {

// These define how big our SmallVectors are, which means
// this should be a good average length we expect to parse
const int COR_IDLIST_SIZE = 5; // list of ids, used by extends, implements
const int COR_FORMAL_PARAM_VECTOR_SIZE = 5; // formal parameters in function/method decl

// a list of symbols, used for extends, implements
typedef llvm::SmallVector<std::string, COR_IDLIST_SIZE> idList;
typedef std::vector<const pSourceRange*> sourceRangeList;

enum nodeKind {
#define STMT(CLASS, PARENT) CLASS##Kind,
#include "corvus/astNodes.def"
};

class stmt;

/* statement iterators */
class stmtIteratorBase {
protected:

  stmt** ptr;
  stmtIteratorBase(stmt** s) : ptr(s) {}
  stmtIteratorBase() : ptr(NULL) {}

};


template <typename DERIVED, typename REFERENCE>
class stmtIteratorImpl : public stmtIteratorBase,
                         public std::iterator<std::forward_iterator_tag,
                                              REFERENCE, ptrdiff_t,
                                              REFERENCE, REFERENCE> {
protected:
  stmtIteratorImpl(const stmtIteratorBase& RHS) : stmtIteratorBase(RHS) {}
public:
  stmtIteratorImpl() {}
  stmtIteratorImpl(stmt** s) : stmtIteratorBase(s) {}

  DERIVED& operator++() {
      ++ptr;
      return static_cast<DERIVED&>(*this);
  }

  DERIVED operator++(int) {
    DERIVED tmp = static_cast<DERIVED&>(*this);
    operator++();
    return tmp;
  }

  bool operator==(const DERIVED& RHS) const { return ptr == RHS.ptr; }
  bool operator!=(const DERIVED& RHS) const { return ptr != RHS.ptr; }
  REFERENCE operator*() const { return (REFERENCE) *ptr; }
  REFERENCE operator->() const { return operator*(); }

};

struct stmtIterator : public stmtIteratorImpl<stmtIterator,stmt*&> {
  explicit stmtIterator() : stmtIteratorImpl<stmtIterator,stmt*&>() {}

  stmtIterator(stmt** S) : stmtIteratorImpl<stmtIterator,stmt*&>(S) {}
};

struct constStmtIterator : public stmtIteratorImpl<constStmtIterator,
                                                   const stmt*> {
  explicit constStmtIterator() :
    stmtIteratorImpl<constStmtIterator,const stmt*>() {}

  constStmtIterator(const stmtIterator& RHS) :
    stmtIteratorImpl<constStmtIterator,const stmt*>(RHS) {}
};

// This macro implements the clone method and llvm::cast<> support (classof methods)
// for leaf nodes.
#define IMPLEMENT_SUPPORT_MEMBERS(CLASS)      virtual CLASS * clone(pParseContext& C) const {\
                                                 return new (C) CLASS(*this, C);\
                                              }\
                                              static bool classof(const CLASS *) { return true; }\
                                              static bool classof(const stmt* s) { return s->kind() == CLASS##Kind; }\
                                              CLASS * retain() { stmt::retain(); return this; }

// statement base class
class stmt {

    nodeKind kind_;

    pUInt refCount_;

    pUInt startLineNum_;
    pUInt endLineNum_;

    // startCol is col @ startLineNum, endCol is col @ endLineNum
    pUInt startCol_;
    pUInt endCol_;

protected:
  void* operator new(size_t bytes) throw() {
    assert(0 && "stmt cannot be allocated with regular 'new'.");
    return 0;
  }
  void operator delete(void* data) throw() {
    assert(0 && "stmt cannot be released with regular 'delete'.");
  }
  void* operator new[](size_t bytes) throw() {
    assert(0 && "stmt cannot be allocated with regular 'new[]'.");
    return 0;
  }
  void operator delete[](void* data) throw() {
    assert(0 && "stmt cannot be released with regular 'delete[]'.");
  }

  // overridden by extending classes to perform class specific destruction
  // note you should still always call Stmt::doDestroy
  virtual void doDestroy(pParseContext& C);

  void destroyChildren(pParseContext& C);
  
  stmt(const stmt& other): kind_(other.kind_), refCount_(1),
          startLineNum_(other.startLineNum_), endLineNum_(other.endLineNum_),
          startCol_(0), endCol_(0){}
    
  // This method assists in deep-copys of stmt**'s which are present for example in block nodes.
  void deepCopyChildren(stmt**& newChildren, stmt** const& oldChildren, pUInt numChildren, pParseContext& C) {
      if(numChildren) {
          newChildren = new (C) stmt*[numChildren];
          for(pUInt i = 0; i < numChildren; ++i) {
              if(oldChildren[i])
                  newChildren[i] = oldChildren[i]->clone(C);
              else
                  newChildren[i] = 0;
          }
      }
  }
public:
    stmt(nodeKind k): kind_(k), refCount_(1), startLineNum_(0), endLineNum_(0),
        startCol_(0), endCol_(0) { }

    void destroy(pParseContext& C) {
        assert(refCount_ >= 1);
        if (--refCount_ == 0)
            doDestroy(C);
    }

    stmt* retain() {
      assert(refCount_ >= 1);
      ++refCount_;
      return this;
    }
    
    pUInt getRefCount() const {
        return refCount_;
    }

    virtual ~stmt(void) { }

    inline void* operator new(size_t bytes, pParseContext& C, size_t align=8) throw() {
        return C.allocate(bytes, align);
    }
    inline void* operator new(size_t bytes, void* mem) throw() {
      return mem;
    }
    void operator delete(void*, pParseContext&, unsigned) throw() { }
    void operator delete(void*, std::size_t) throw() { }
    void operator delete(void*, void*) throw() { }

    typedef stmtIterator       child_iterator;
    typedef constStmtIterator  const_child_iterator;
    typedef boost::iterator_range<child_iterator> child_range;
    typedef boost::iterator_range<const_child_iterator> const_child_range;


    virtual child_iterator child_begin() = 0;
    virtual child_iterator child_end()   = 0;

    const_child_iterator child_begin() const {
      return const_child_iterator(const_cast<stmt*>(this)->child_begin());
    }

    const_child_iterator child_end() const {
      return const_child_iterator(const_cast<stmt*>(this)->child_end());
    }
    child_range children() {
        return child_range(child_begin(), child_end());
    }
    const_child_range children() const {
        return const_child_range(child_begin(), child_end());
    }

    nodeKind kind(void) const { return kind_; }

    void setLine(pUInt start) { startLineNum_ = start; endLineNum_ = start; }
    // simple case where one token defines col range
    void setCol(pColRange cols) { startCol_ = cols.first; endCol_ = cols.second; }
    // start/end on multiple lines so cols come from different tokens
    // this presumes startCols was generated from the token on startLine and endCols
    // was generated from the token on endLine
    void setCol(pColRange startCols, pColRange endCols) {
        startCol_ = startCols.first;
        endCol_ = endCols.second;
    }
    void setLine(pUInt start, pUInt end) { startLineNum_ = start; endLineNum_ = end; }

    pUInt startLineNum(void) const { return startLineNum_; }
    pUInt endLineNum(void) const { return endLineNum_; }
    pUInt startCol(void) const { return startCol_; }
    pUInt endCol(void) const { return endCol_; }
    pColRange cols(void) const { return pColRange(startCol_, endCol_); }

    // Polymorphic deep copying.
    virtual stmt* clone(pParseContext& C) const = 0;

    // LLVM isa<T> and casting support
    static bool classof(const stmt* s) { return true; }

};

typedef std::vector<stmt*> statementList;

// expression base class
class expr: public stmt {

public:
    // see astNodes.def
    static const nodeKind firstExprKind = assignmentKind;
    static const nodeKind lastExprKind = unaryOpKind;

    expr(nodeKind k): stmt(k) { }

    static bool classof(const expr* s) { return true; }
    static bool classof(const stmt* s) {
        return s->kind() >= firstExprKind &&
               s->kind() <= lastExprKind;
    }
    
    // We include that clone here so we get a clone which returns expr*.
    virtual expr* clone(pParseContext& C) const = 0;
    expr* retain() { stmt::retain(); return this; }
    
protected:
    expr(const expr& other): stmt(other) {}
};

typedef std::vector<expr*> expressionList;

// a block of statements
class block: public stmt {
    stmt** block_;
    pUInt numStmts_;
    
protected:
    block(const block& other, pParseContext& C): stmt(other), block_(0), 
            numStmts_(other.numStmts_)
    {
    	deepCopyChildren(block_, other.block_, numStmts_, C);
    }
public:

    // build a block with a single expression
    block(pParseContext& C, stmt* s): stmt(blockKind), block_(0), numStmts_(1) {
        block_ = new (C) stmt*[numStmts_];
        block_[0] = s;
    }

    block(pParseContext& C, const statementList* s): stmt(blockKind), block_(0), numStmts_(s->size()) {
        if (numStmts_) {
            block_ = new (C) stmt*[numStmts_];
            memcpy(block_, &(s->front()), numStmts_ * sizeof(*block_));
        }
    }

    block(pParseContext& C, const expressionList* s): stmt(blockKind), block_(0), numStmts_(s->size()) {
        if (numStmts_) {
            block_ = new (C) stmt*[numStmts_];
            memcpy(block_, &(s->front()), numStmts_ * sizeof(*block_));
        }
    }

    stmt::child_iterator child_begin() { return &block_[0]; }
    stmt::child_iterator child_end() { return &block_[0]+numStmts_; }

    IMPLEMENT_SUPPORT_MEMBERS(block);

};

// declaration base class
class decl: public stmt {
    
protected:
    decl(const decl& other, pParseContext& C): stmt(other) {}
    
public:
    // see astNodes.def
    static const nodeKind firstDeclKind = useDeclKind;
    static const nodeKind lastDeclKind = functionDeclKind;

    decl(nodeKind k): stmt(k) { }

    static bool classof(const decl* s) { return true; }
    static bool classof(const stmt* s) {
        return s->kind() >= firstDeclKind &&
               s->kind() <= lastDeclKind;
    }

    // We do not implement clone here because decl is an abstract base.
};


// global
class globalDecl: public decl {

    stmt** children_;
    pUInt numChildren_;

protected:
    globalDecl(const globalDecl& other, pParseContext& C): decl(other), children_(0),
        numChildren_(other.numChildren_)
    {
        deepCopyChildren(children_, other.children_, numChildren_, C);
    }
public:

    globalDecl(const expressionList* varList,
               pParseContext& C):
            decl(globalDeclKind),
            children_(0),
            numChildren_(varList->size())
    {
        children_ = new (C) stmt*[numChildren_];
        memcpy(children_, &(varList->front()), (numChildren_) * sizeof(stmt*));
    }

    stmt::child_iterator child_begin() { return &children_[0]; }
    stmt::child_iterator child_end() { return &children_[0]+numChildren_; }

    IMPLEMENT_SUPPORT_MEMBERS(globalDecl);

};

class staticDecl: public decl {

    enum { DEFAULT=0, VARS=1 };

    stmt** children_;
    pUInt numChildren_;

protected:
    staticDecl(const staticDecl& other, pParseContext& C): decl(other), children_(0),
        numChildren_(other.numChildren_)
    {
        deepCopyChildren(children_, other.children_, numChildren_, C);
    }
    
public:
    staticDecl(const expressionList* varList,
               pParseContext& C,
               expr* def=NULL):
        decl(staticDeclKind),
        children_(0),
        numChildren_(1+varList->size())
    {
        children_ = new (C) stmt*[numChildren_];
        children_[DEFAULT] = def;
        if (numChildren_ > 1) {
            memcpy(children_+1, &(varList->front()), (numChildren_-1) * sizeof(stmt*));
        }
    }

    expr* defaultExpr(void) {
        return static_cast<expr*>(children_[DEFAULT]);
    }

    stmt::child_iterator child_begin() { return &children_[DEFAULT]; }
    stmt::child_iterator child_end() { return &children_[DEFAULT]+numChildren_; }

    stmt::child_iterator var_begin() { return &children_[VARS]; }
    stmt::child_iterator var_end() { return &children_[VARS]+(numChildren_-1); }

    IMPLEMENT_SUPPORT_MEMBERS(staticDecl);

};



class namespaceName {
public:
    typedef std::vector<std::string> partsType;

protected:
    pColRange cols_;
    partsType parts_;
    bool absolute_;

public:
    namespaceName(int startCol): absolute_(false) {
        cols_.first = startCol;
    }

    void setEndCol(int endCol) {
        cols_.second = endCol;
    }

    pColRange cols() const { return cols_; }

    void push_back(pStringRef part) {
        parts_.push_back(part);
    }

    std::string getPrimary() const {
        return parts_.back();
    }

    void setAbsolute() { absolute_ = true; }

    std::string getFullName() const {
        std::string result;
        if (absolute_)
            result.push_back('\\');
        result.append(parts_[0]);
        if (parts_.size() > 1) {
            result.push_back('\\');
            for (int i = 1; i < parts_.size(); i++) {
                result.append(parts_[i]);
                if (i < parts_.size()-1)
                    result.push_back('\\');
            }
        }
        return result;
    }

};

typedef llvm::SmallVector<const namespaceName*, COR_IDLIST_SIZE> namespaceList;

class namespaceDecl: public decl {

    std::string name_;

protected:
    namespaceDecl(const namespaceDecl& other, pParseContext& C): decl(other),
        name_(other.name_) { }

public:
    namespaceDecl(const namespaceName* ns, pParseContext& C):
        decl(namespaceDeclKind) {
        name_ = ns->getFullName();
    }

    pStringRef name(void) const {
        return name_;
    }

    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    IMPLEMENT_SUPPORT_MEMBERS(namespaceDecl);

};

class useIdent;
typedef std::vector<useIdent*> useIdentList;

class useDecl: public decl {

    stmt** children_;
    pUInt numChildren_;

protected:
    useDecl(const useDecl& other, pParseContext& C): decl(other), children_(0),
        numChildren_(other.numChildren_)
    {
        deepCopyChildren(children_, other.children_, numChildren_, C);
    }
public:

    useDecl(const useIdentList* identList,
               pParseContext& C):
            decl(useDeclKind),
            children_(0),
            numChildren_(identList->size())
    {
        children_ = new (C) stmt*[numChildren_];
        memcpy(children_, &(identList->front()), (numChildren_) * sizeof(decl*));
    }

    stmt::child_iterator child_begin() { return &children_[0]; }
    stmt::child_iterator child_end() { return &children_[0]+numChildren_; }

    IMPLEMENT_SUPPORT_MEMBERS(useDecl);

};


class useIdent: public decl {

    std::string nsname_;
    std::string alias_;

protected:
    useIdent(const useIdent& other, pParseContext& C): decl(other),
        nsname_(other.nsname_), alias_(other.alias_) { }

public:
    useIdent(const namespaceName* ns, pParseContext& C):
        decl(useIdentKind) {
        nsname_ = ns->getFullName();
        alias_.clear();
    }
    useIdent(const namespaceName* ns, const pSourceRange& alias, pParseContext& C):
        decl(useIdentKind) {
        nsname_ = ns->getFullName();
        alias_ = alias;
    }

    pStringRef nsname(void) const {
        return nsname_;
    }

    pStringRef alias(void) const {
        return alias_;
    }

    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    IMPLEMENT_SUPPORT_MEMBERS(useIdent);

};

class formalParam: public decl {

    enum { byRefBit=1 };

    std::string name_;
    std::string hint_;
    pUInt flags_;
    stmt* default_;

    formalParam(const formalParam& other, pParseContext& C): decl(other),
            name_(other.name_), hint_(other.hint_), flags_(other.flags_)
    {
        if(other.default_)
            default_ = other.default_->clone(C);
    }
public:
    formalParam(const pSourceRange& name, pParseContext& C, bool ref, expr* def=NULL):
        decl(formalParamKind),
        name_(name),
        hint_(),
        flags_(0),
        default_(def)
    {
        if (ref) {
            flags_ ^= byRefBit;
        }
    }

    bool hasDefault(void) {
        return defaultExpr() != NULL;
    }

    expr* defaultExpr(void) {
        return static_cast<expr*>(default_);
    }

    bool byRef(void) const { return flags_ & byRefBit; }

    bool optional(void) const {  return default_ != NULL; }

    void setHint(const pStringRef name) {
        hint_ = name;
    }

    pStringRef name(void) const {
        return name_;
    }

    pStringRef hint(void) const {
        if (!hint_.empty())
            return hint_;
        else
            return "";
    }

    stmt::child_iterator child_begin() { return &default_; }
    stmt::child_iterator child_end() { return &default_+1; }

    IMPLEMENT_SUPPORT_MEMBERS(formalParam);

};

typedef llvm::SmallVector<formalParam*,COR_FORMAL_PARAM_VECTOR_SIZE> formalParamList;

class signature: public decl {

    bool anonymous_;
    std::string name_;
    stmt** formalParamList_;
    stmt** useParamList_;
    pUInt numParams_;
    pUInt numUseParams_;
    bool returnByRef_;

protected:
    signature(const signature& other, pParseContext& C): decl(other), name_(other.name_),
        formalParamList_(0), numParams_(other.numParams_), useParamList_(0), numUseParams_(other.numUseParams_),
        returnByRef_(other.returnByRef_)
    {
        deepCopyChildren(formalParamList_, other.formalParamList_, numParams_, C);
        deepCopyChildren(useParamList_, other.useParamList_, numUseParams_, C);
    }
    
public:
    signature(const pSourceRange& name,
              pParseContext& C,
              const formalParamList* s,
              bool returnByRef=false):
            decl(signatureKind),
            name_(name),
            formalParamList_(0),
            useParamList_(0),
            numParams_(s ? s->size() : 0),
            numUseParams_(0),
            returnByRef_(returnByRef),
            anonymous_(false)
    {
        if (numParams_) {
            formalParamList_ = new (C) stmt*[numParams_];
            memcpy(formalParamList_, &(s->front()), numParams_ * sizeof(*formalParamList_));
        }
    }

    signature(pParseContext& C,
              const formalParamList* s,
              const formalParamList* s2=NULL
              ):
            decl(signatureKind),
            formalParamList_(0),
            numParams_(s ? s->size() : 0),
            useParamList_(0),
            numUseParams_(s2 ? s2->size() : 0),
            returnByRef_(false),
            anonymous_(true)
    {
        if (numParams_) {
            formalParamList_ = new (C) stmt*[numParams_];
            memcpy(formalParamList_, &(s->front()), numParams_ * sizeof(*formalParamList_));
        }
        if (numUseParams_) {
            useParamList_ = new (C) stmt*[numUseParams_];
            memcpy(useParamList_, &(s->front()), numUseParams_ * sizeof(*useParamList_));
        }
    }


    pStringRef name(void) const {
        return name_;
    }

    bool returnByRef(void) const { return returnByRef_; }

    pUInt numParams(void) const { return numParams_; }

    pUInt numUseParams(void) const { return numUseParams_; }

    formalParam* getParam(uint i) {
        return static_cast<formalParam*>(formalParamList_[i]);
    }

    formalParam* getUseParam(uint i) {
        return static_cast<formalParam*>(useParamList_[i]);
    }

    stmt::child_iterator child_begin() { return &formalParamList_[0]; }
    stmt::child_iterator child_end() { return &formalParamList_[0]+numParams_; }

    stmt::child_iterator use_begin() { return &useParamList_[0]; }
    stmt::child_iterator use_end() { return &useParamList_[0]+numUseParams_; }

    IMPLEMENT_SUPPORT_MEMBERS(signature);
};


struct memberFlags {
    // these are taken by address during parse
    static const pUInt PUBLIC;
    static const pUInt PROTECTED;
    static const pUInt PRIVATE;
    static const pUInt STATIC;
    static const pUInt ABSTRACT;
    static const pUInt FINAL;
    static const pUInt CONST;
};

// method declaration
class methodDecl: public decl {

    enum { SIG, BODY, END_EXPR };
    pUInt flags_;
    stmt* children_[END_EXPR];

protected:
    methodDecl(const methodDecl& other, pParseContext& C): decl(other),
        flags_(other.flags_)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[SIG])
            children_[SIG] = other.children_[SIG]->clone(C);
        if(other.children_[BODY])
            children_[BODY] = other.children_[BODY]->clone(C);
    }
    
public:
    methodDecl(signature* sig, pUInt flags, block* body):
        decl(methodDeclKind),
        flags_(flags),
        children_()
    {
        children_[SIG] = sig;
        children_[BODY] = body; // body may be null for abstract method
        if (body == NULL)
            flags_ |= memberFlags::ABSTRACT;
    }

    signature* sig(void) { return static_cast<signature*>(children_[SIG]); }
    block* body(void) { return static_cast<block*>(children_[BODY]); }
    pUInt flags(void) const { return flags_; }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    IMPLEMENT_SUPPORT_MEMBERS(methodDecl);

};

// property declaration
class propertyDecl: public decl {

    pUInt flags_;
    std::string name_;
    expr* default_;
    
protected:
    propertyDecl(const propertyDecl& other, pParseContext& C): decl(other),
        flags_(other.flags_), name_(other.name_), default_(0)
    {
        if(other.default_)
            default_ = other.default_->clone(C);
    }
    
public:
    propertyDecl(pParseContext& C,
                 const pSourceRange& name,
                 expr* def
                 ):
        decl(propertyDeclKind),
        flags_(0),
        name_(name),
        default_(def)
    {
    }

    void setFlags(pUInt f) { flags_ = f; }
    pUInt flags(void) const { return flags_; }
    pStringRef name(void) const {
        return name_;
    }

    stmt::child_iterator child_begin() { return (stmt**)&default_; }
    stmt::child_iterator child_end() { return (stmt**)&default_+1; }

    IMPLEMENT_SUPPORT_MEMBERS(propertyDecl);
};



// class/interface declaration
class classDecl: public decl {
public:

    enum classTypes { NORMAL,
                      IFACE, // "INTERFACE" is keyword? throws error
                      FINAL,
                      ABSTRACT };

private:
    std::string name_;
    idList extends_;
    idList implements_;
    classTypes classType_;
    block* members_;
    
protected:
    // We copy the SmallVectors here, this works for PooledStringPtrs but wouldn't for 
    // stmt*s!
    classDecl(const classDecl& other, pParseContext& C): decl(other),
        name_(other.name_), extends_(other.extends_), implements_(other.implements_),
        classType_(other.classType_), members_(0)
    {
        if(other.members_)
            members_ = other.members_->clone(C);
    }
    
public:
    classDecl(pParseContext& C,
              const pSourceRange& name,
              classTypes type,
              const namespaceList* extends, // may be null
              const namespaceList* implements, // may be null
              block* members
              ):
        decl(classDeclKind),
        name_(name),
        extends_(),
        implements_(),
        classType_(type),
        members_(members)
    {
        // intern list of extends (if any)
        if (extends) {
            for (namespaceList::const_iterator i=extends->begin();
            i != extends->end();
            ++i) {
                extends_.push_back((*i)->getFullName());
                delete (*i);
            }
            delete extends;
        }
        // intern list of implements (if any)
        if (implements) {
            for (namespaceList::const_iterator i=implements->begin();
            i != implements->end();
            ++i) {
                implements_.push_back((*i)->getFullName());
                delete (*i);
            }
            delete implements;
        }
    }

    pStringRef name(void) const {
        return name_;
    }

    block* members(void) { return members_; }

    classTypes classType(void) const { return classType_; }

    pUInt implementsCount(void) const { return implements_.size(); }
    pUInt extendsCount(void) const { return extends_.size(); }

    stmt::child_iterator child_begin() { return (stmt**)&members_; }
    stmt::child_iterator child_end() { return (stmt**)&members_+1; }

    idList::iterator extends_begin() { return extends_.begin(); }
    idList::iterator extends_end() { return extends_.end(); }

    idList::iterator implements_begin() { return implements_.begin(); }
    idList::iterator implements_end() { return implements_.end(); }

    IMPLEMENT_SUPPORT_MEMBERS(classDecl);

};

// if statement
class ifStmt: public stmt {

    enum { CONDITION, TRUEBLOCK, FALSEBLOCK, END_EXPR };
    stmt* children_[END_EXPR];

protected:
    ifStmt(const ifStmt& other, pParseContext& C): stmt(other)
    {
        memset(children_, 0, sizeof(children_));
        children_[CONDITION] = other.children_[CONDITION]->clone(C);
        if(other.children_[TRUEBLOCK])
            children_[TRUEBLOCK] = other.children_[TRUEBLOCK]->clone(C);
        if(other.children_[FALSEBLOCK])
            children_[FALSEBLOCK] = other.children_[FALSEBLOCK]->clone(C);
    }
    
public:
    ifStmt(pParseContext& C,
           expr* cond,
           stmt* trueBlock,
           stmt* falseBlock):
               stmt(ifStmtKind),
               children_() {

        // enforce blocks to ease later traversal
        if (trueBlock && !isa<block>(trueBlock))
            trueBlock = new (C) block(C, trueBlock);

        if (falseBlock && !isa<block>(falseBlock))
            falseBlock = new (C) block(C, falseBlock);

        children_[CONDITION] = static_cast<stmt*>(cond);
        children_[TRUEBLOCK] = trueBlock;
        children_[FALSEBLOCK] = falseBlock;

    }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    expr* condition(void) { return static_cast<expr*>(children_[CONDITION]); }
    stmt* trueBlock(void) { return children_[TRUEBLOCK]; }
    stmt* falseBlock(void) { return children_[FALSEBLOCK]; }

    IMPLEMENT_SUPPORT_MEMBERS(ifStmt);

};

// switch case
class switchCase: public stmt {

    enum { COND, BODY, END_EXPR };
    stmt* children_[END_EXPR];

protected:
    switchCase(const switchCase& other, pParseContext& C): stmt(other)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[COND])
            children_[COND] = other.children_[COND]->clone(C);
        if(other.children_[BODY])
            children_[BODY] = other.children_[BODY]->clone(C);
    }
    
public:
    switchCase(expr* cond,
               block* body):
    stmt(switchCaseKind),
    children_()
    {

        children_[COND] = static_cast<stmt*>(cond);
        children_[BODY] = static_cast<stmt*>(body);

    }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    // note if condition is null, this is a default: case
    expr* condition(void) { return static_cast<expr*>(children_[COND]); }
    block* body(void) { return static_cast<block*>(children_[BODY]); }

    IMPLEMENT_SUPPORT_MEMBERS(switchCase);

};

// switch statement
class switchStmt: public stmt {

    enum { RVAL, CASEBLOCK, END_EXPR };
    stmt* children_[END_EXPR];

protected:
    switchStmt(const switchStmt& other, pParseContext& C): stmt(other)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[RVAL])
            children_[RVAL] = other.children_[RVAL]->clone(C);
        if(other.children_[CASEBLOCK])
            children_[CASEBLOCK] = other.children_[CASEBLOCK]->clone(C);
    }
    
public:
    switchStmt(expr* rVal,
               block* caseBlock):
    stmt(switchStmtKind),
    children_() {

        children_[RVAL] = static_cast<stmt*>(rVal);
        children_[CASEBLOCK] = caseBlock;

    }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    expr* rVal(void) { return static_cast<expr*>(children_[RVAL]); }
    block* caseBlock(void) { return static_cast<block*>(children_[CASEBLOCK]); }

    IMPLEMENT_SUPPORT_MEMBERS(switchStmt);

};

// foreach statement
class forEach: public stmt {

    enum { RVAL, KEY, VAL, BODY, END_EXPR };
    stmt* children_[END_EXPR];
    bool byRef_; // value

protected:
    forEach(const forEach& other, pParseContext& C): stmt(other), byRef_(other.byRef_)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[RVAL])
            children_[RVAL] = other.children_[RVAL]->clone(C);
        if(other.children_[KEY])
            children_[KEY] = other.children_[KEY]->clone(C);
        if(other.children_[VAL])
            children_[VAL] = other.children_[VAL]->clone(C);
        if(other.children_[BODY])
            children_[BODY] = other.children_[BODY]->clone(C);
    }
    
public:
    forEach(expr* rVal,
            stmt* body,
            pParseContext& C,
            expr* val,
            bool byRef,
            expr* key=NULL):
    stmt(forEachKind),
    children_(),
    byRef_(byRef)
    {

        // enfore a block for body to ease later traversal
        if (!isa<block>(body)) {
            body = new (C) block(C, body);
        }

        children_[RVAL] = static_cast<stmt*>(rVal);
        children_[KEY] = static_cast<stmt*>(key);
        children_[VAL] = static_cast<stmt*>(val);
        children_[BODY] = static_cast<stmt*>(body);

    }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    expr* rVal(void) { return static_cast<expr*>(children_[RVAL]); }
    expr* key(void) { return static_cast<expr*>(children_[KEY]); }
    expr* val(void) { return static_cast<expr*>(children_[VAL]); }
    block* body(void) { return static_cast<block*>(children_[BODY]); }

    bool hasKey(void) const { return (bool)children_[KEY]; }

    IMPLEMENT_SUPPORT_MEMBERS(forEach);

};

// for statement
class forStmt: public stmt {

    enum { INIT, COND, INC, BODY, END_EXPR };
    stmt* children_[END_EXPR];

protected:
    forStmt(const forStmt& other, pParseContext& C): stmt(other)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[INIT])
            children_[INIT] = other.children_[INIT]->clone(C);
        if(other.children_[COND])
            children_[COND] = other.children_[COND]->clone(C);
        if(other.children_[INC])
            children_[INC] = other.children_[INC]->clone(C);
        if(other.children_[BODY])
            children_[BODY] = other.children_[BODY]->clone(C);
    }

public:
    forStmt(pParseContext& C,
            stmt* init,
            stmt* cond,
            stmt* inc,
            stmt* body):
    stmt(forStmtKind),
    children_()
    {

        // enfore a block for body to ease later traversal
        if (!isa<block>(body)) {
            body = new (C) block(C, body);
        }

        children_[INIT] = init; // may be block or expr
        children_[COND] = cond; // may be block or expr
        children_[INC] = inc; // may be block or expr
        children_[BODY] = static_cast<stmt*>(body);

    }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    stmt* init(void) const { return children_[INIT]; }
    stmt* condition(void) const { return children_[COND]; }
    stmt* increment(void) const { return children_[INC]; }
    block* body(void) const { return static_cast<block*>(children_[BODY]); }

    IMPLEMENT_SUPPORT_MEMBERS(forStmt);
};

// do statement
class doStmt: public stmt {

    enum { COND, BODY, END_EXPR };
    stmt* children_[END_EXPR];

protected:
    doStmt(const doStmt& other, pParseContext& C): stmt(other)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[COND])
            children_[COND] = other.children_[COND]->clone(C);
        if(other.children_[BODY])
            children_[BODY] = other.children_[BODY]->clone(C);
    }

public:
    doStmt(pParseContext& C,
           expr* cond,
           stmt* body):
    stmt(doStmtKind),
    children_()
    {

        // enfore a block for body to ease later traversal
        if (!isa<block>(body)) {
            body = new (C) block(C, body);
        }

        children_[COND] = static_cast<stmt*>(cond);
        children_[BODY] = static_cast<stmt*>(body);

    }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    expr* condition(void) { return static_cast<expr*>(children_[COND]); }
    block* body(void) { return static_cast<block*>(children_[BODY]); }

    IMPLEMENT_SUPPORT_MEMBERS(doStmt);

};

// while statement
class whileStmt: public stmt {

    enum { COND, BODY, END_EXPR };
    stmt* children_[END_EXPR];

protected:
    whileStmt(const whileStmt& other, pParseContext& C): stmt(other)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[COND])
            children_[COND] = other.children_[COND]->clone(C);
        if(other.children_[BODY])
            children_[BODY] = other.children_[BODY]->clone(C);
    }

public:
    whileStmt(pParseContext& C,
              expr* cond,
              stmt* body):
    stmt(whileStmtKind),
    children_()
    {

        // enfore a block for body to ease later traversal
        if (!isa<block>(body)) {
            body = new (C) block(C, body);
        }

        children_[COND] = static_cast<stmt*>(cond);
        children_[BODY] = static_cast<stmt*>(body);

    }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    expr* condition(void) { return static_cast<expr*>(children_[COND]); }
    block* body(void) { return static_cast<block*>(children_[BODY]); }

    IMPLEMENT_SUPPORT_MEMBERS(whileStmt);

};



// type cast
class typeCast: public expr {
public:
    enum castKindType { STRING, BINARY, UNICODE, INT, REAL, BOOL, UNSET, ARRAY, OBJECT };

private:
    expr* rVal_;
    castKindType castKind_;

protected:
    typeCast(const typeCast& other, pParseContext& C): expr(other),
            rVal_(0), castKind_(other.castKind_)
    {
        if(other.rVal_)
            rVal_ = other.rVal_->clone(C);
    }

public:
    typeCast(castKindType kind, expr* rVal): expr(typeCastKind), rVal_(rVal), castKind_(kind)
    {

    }

    stmt::child_iterator child_begin() { return reinterpret_cast<stmt**>(&rVal_); }
    stmt::child_iterator child_end() { return reinterpret_cast<stmt**>(&rVal_+1); }

    expr* rVal(void) { return rVal_; }
    castKindType castKind(void) const { return castKind_; }

    IMPLEMENT_SUPPORT_MEMBERS(typeCast);

};

// return statement
class returnStmt: public stmt {
    
    expr* rVal_;
    
protected:
    returnStmt(const returnStmt& other, pParseContext& C): stmt(other), rVal_(0)
    {
        if(other.rVal_)
            rVal_ = other.rVal_->clone(C);
    }

public:
    returnStmt(expr* rVal): stmt(returnStmtKind), rVal_(rVal)
    {

    }

    stmt::child_iterator child_begin() { return reinterpret_cast<stmt**>(&rVal_); }
    stmt::child_iterator child_end() { return reinterpret_cast<stmt**>(&rVal_+1); }

    expr* rVal(void) { return rVal_; }
    void setRVal(pParseContext& C, expr* r) { if(rVal_) rVal_->destroy(C); rVal_= r; }

    IMPLEMENT_SUPPORT_MEMBERS(returnStmt);

};

// break statement
class breakStmt: public stmt {
    
    expr* rVal_;
    
protected:
    breakStmt(const breakStmt& other, pParseContext& C): stmt(other), rVal_(0)
    {
        if(other.rVal_)
            rVal_ = other.rVal_->clone(C);
    }

public:
    breakStmt(expr* rVal): stmt(breakStmtKind), rVal_(rVal)
    {

    }

    stmt::child_iterator child_begin() { return reinterpret_cast<stmt**>(&rVal_); }
    stmt::child_iterator child_end() { return reinterpret_cast<stmt**>(&rVal_+1); }

    expr* rVal(void) { return rVal_; }

    IMPLEMENT_SUPPORT_MEMBERS(breakStmt);

};

// continue statement
class continueStmt: public stmt {
    
    expr* rVal_;
    
protected:
    continueStmt(const continueStmt& other, pParseContext& C): stmt(other), rVal_(0)
    {
        if(other.rVal_)
            rVal_ = other.rVal_->clone(C);
    }

public:
    continueStmt(expr* rVal): stmt(continueStmtKind), rVal_(rVal)
    {

    }

    stmt::child_iterator child_begin() { return reinterpret_cast<stmt**>(&rVal_); }
    stmt::child_iterator child_end() { return reinterpret_cast<stmt**>(&rVal_+1); }

    expr* rVal(void) { return rVal_; }

    IMPLEMENT_SUPPORT_MEMBERS(continueStmt);

};

// catch
class catchStmt: public stmt {

    std::string className_;
    std::string varName_;
    block* body_;
    
protected:
    catchStmt(const catchStmt& other, pParseContext& C): stmt(other),
        className_(other.className_), varName_(other.varName_), body_(0)
    {
        if(other.body_)
            body_ = other.body_->clone(C);
    }

public:
    catchStmt(const namespaceName* className,
              const pSourceRange& varName,
              pParseContext& C,
              block* body):
    stmt(catchStmtKind),
    className_(className->getFullName()),
    varName_(pStringRef(varName.begin(), (varName.end()-varName.begin()))),
    body_(body)
    {
    }

    pStringRef className(void) const {
        return className_;
    }

    pStringRef varName(void) const {
        return varName_;
    }

    stmt::child_iterator child_begin() { return (stmt**)&body_; }
    stmt::child_iterator child_end() { return (stmt**)&body_+1; }

    IMPLEMENT_SUPPORT_MEMBERS(catchStmt);

};


// try
class tryStmt: public stmt {

    enum { BODY=0, CATCHLIST=1 };

    // children_[0] is always body. the rest are catch blocks
    // numChildren_ is always 1 + number of catch blocks
    stmt** children_;
    pUInt numChildren_;

protected:
    tryStmt(const tryStmt& other, pParseContext& C): stmt(other), children_(0),
        numChildren_(other.numChildren_)
    {
        deepCopyChildren(children_, other.children_, numChildren_, C);
    }
    
public:
    tryStmt(pParseContext& C, block* body, statementList* catchList):
        stmt(tryStmtKind),
        children_(NULL),
        numChildren_(1+catchList->size())
    {
        children_ = new (C) stmt*[numChildren_];
        children_[BODY] = body;
        if (numChildren_ > 1) {
            memcpy(children_+1, &(catchList->front()), (numChildren_-1) * sizeof(stmt*));
        }
    }

    block* body(void) {
        return static_cast<block*>(children_[BODY]);
    }

    stmt::child_iterator child_begin() { return &children_[0]; }
    stmt::child_iterator child_end() { return &children_[0]+numChildren_; }

    pUInt numCatches(void) const { return numChildren_-1; }

    stmt::child_iterator catches_begin() { return &children_[CATCHLIST]; }
    stmt::child_iterator catches_end() { return &children_[CATCHLIST]+(numChildren_-1); }

    IMPLEMENT_SUPPORT_MEMBERS(tryStmt);

};


// builtins: language constructs that seem like function calls, but aren't.
// exit, isset, unset, empty, echo, print, include, require, clone, throw, @
class builtin: public expr {
public:
    enum opKind {
                  EXIT, // start expr
                  ISSET,
                  UNSET,
                  EMPTY,
                  CLONE,
                  INCLUDE,
                  INCLUDE_ONCE,
                  REQUIRE,
                  REQUIRE_ONCE,
                  PRINT, // end expr
                  ECHO,   // start statements
                  IGNORE_WARNING,
                  THROW   // end statements
                };

private:
    stmt** children_;
    pUInt numChildren_;
    opKind opKind_;

protected:
    builtin(const builtin& other, pParseContext& C): expr(other), children_(0),
        numChildren_(other.numChildren_), opKind_(other.opKind_)
    {
        deepCopyChildren(children_, other.children_, numChildren_, C);
    }
    
public:

    builtin(pParseContext& C, opKind op, const expressionList* s=NULL):
            expr(builtinKind),
            children_(0),
            numChildren_(0),
            opKind_(op)
    {
        if (s) {
            numChildren_ = s->size();
            children_ = new (C) stmt*[numChildren_];
            memcpy(children_, &(s->front()), numChildren_ * sizeof(*children_));
        }
    }

    opKind opKind() const { return opKind_; }
    pUInt numArgs() const { return numChildren_; }

    stmt::child_iterator child_begin() { return &children_[0]; }
    stmt::child_iterator child_end() { return &children_[0]+numChildren_; }

    IMPLEMENT_SUPPORT_MEMBERS(builtin);

};

// literal expression base class
//TODO: do we want literalExpr to be abstract?
class literalExpr: public expr {

protected:
    std::string stringVal_;
    literalExpr(const literalExpr& other, pParseContext& C): expr(other),
            stringVal_(other.stringVal_) {}
    
public:

    // see astNodes.def
    static const nodeKind firstLiteralKind = literalStringKind;
    static const nodeKind lastLiteralKind = inlineHtmlKind;

    literalExpr(nodeKind k): expr(k), stringVal_() { }
    literalExpr(nodeKind k, const pSourceRange& v): expr(k), stringVal_(v.begin(), v.end()-v.begin()) { }

    virtual const pStringRef getStringVal(void) const {
        return stringVal_;
    }

    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    static bool classof(const literalExpr* s) { return true; }
    static bool classof(const stmt* s) {
        return s->kind() >= firstLiteralKind &&
               s->kind() <= lastLiteralKind;
    }
    
    virtual literalExpr* clone(pParseContext& C) const {
        return new (C) literalExpr(*this, C);
    }
    
    literalExpr* retain() { stmt::retain(); return this; }

};

// NODE: literal bstring
class literalString: public literalExpr {

    bool isSimple_; // i.e., single quoted

protected:
    literalString(const literalString& other, pParseContext& C): literalExpr(other),
            isSimple_(other.isSimple_)
    {

    }
    
public:

    // empty source string
    literalString(void):
            literalExpr(literalStringKind),
            isSimple_(true) { }

    // normal source string
    literalString(const pSourceRange& v):
            literalExpr(literalStringKind, v),
            isSimple_(true) { }

    // extending string (inline html)
    literalString(const pSourceRange& v, nodeKind k):
            literalExpr(k, v),
            isSimple_(true) { }

    ~literalString(void) {
    }

    void setIsSimple(bool s)  {
        isSimple_ = s;
    }

    bool isSimple(void) const { return isSimple_; }

    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    IMPLEMENT_SUPPORT_MEMBERS(literalString);

};

// NODE: literal int
class literalInt: public literalExpr {

    //TODO: use bitfield here
    bool negative_;
    bool isParsed_;
    pInt val_;

    void parse() {
        llvm::APInt result;
        if (pStringRef(stringVal_).getAsInteger(0, result))
            val_ = result.getLimitedValue();
        else
            val_ = 0;
        isParsed_ = true;
    }
protected:
    literalInt(const literalInt& other, pParseContext& C): literalExpr(other),
            negative_(other.negative_), isParsed_(other.isParsed_),
            val_(other.val_)
    {}
    
public:
    literalInt(const pSourceRange& v): literalExpr(literalIntKind, v), negative_(false),
            isParsed_(false), val_(0)
    {}

    bool negative(void) const { return negative_; }
    void setNegative(bool n) { negative_ = n; isParsed_ = false;}

    pInt getInt() { if(!isParsed_) parse(); return val_; }
    
    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    IMPLEMENT_SUPPORT_MEMBERS(literalInt);

};

// NODE: literal float
class literalFloat: public literalExpr {

protected:
    literalFloat(const literalFloat& other, pParseContext& C): literalExpr(other) {}
    
public:
    literalFloat(const pSourceRange& v): literalExpr(literalFloatKind, v) { }

    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    IMPLEMENT_SUPPORT_MEMBERS(literalFloat);

};

// NODE: literal bool
class literalBool: public literalExpr {

    bool boolVal_;

protected:
    literalBool(const literalBool& other, pParseContext& C): literalExpr(other),
            boolVal_(other.boolVal_) {}
    
public:
    literalBool(bool v): literalExpr(literalBoolKind), boolVal_(v) { }

    bool getBoolVal(void) const { return boolVal_; }

    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    IMPLEMENT_SUPPORT_MEMBERS(literalBool);

};

// array items
struct arrayItem {
    expr* key;
    expr* val;
    bool isRef;
    arrayItem(expr* k, expr* v, bool r):
     key(k),
     val(v),
     isRef(r)
     { }
};

typedef std::vector<arrayItem> arrayList;

// NODE: literal array
class literalArray: public literalExpr {

    arrayList itemList_;

protected:
    literalArray(const literalArray& other, pParseContext& C): literalExpr(other),
            itemList_(other.itemList_)
    {
        // We have copied all array items by value, now we have to deep-copy the
        // expressions in the array items.
        foreach(arrayItem& item, itemList_) {
            if(item.key)
                item.key = item.key->clone(C);
            if(item.val)
                item.val = item.val->clone(C);
        }
    }
public:
    literalArray(arrayList* items):
        literalExpr(literalArrayKind),
        itemList_(*items) // copy
        { }

    virtual void doDestroy(pParseContext& C) {
        for (arrayList::iterator i = itemList_.begin(); i != itemList_.end(); ++i) {
            if (i->key) {
                i->key->destroy(C);
            }
            i->val->destroy(C);
        }
        stmt::doDestroy(C);
    }

    arrayList& itemList(void) { return itemList_; }
    const arrayList& itemList(void) const { return itemList_; }

    IMPLEMENT_SUPPORT_MEMBERS(literalArray);

};


// NODE: inline html
class inlineHtml: public literalString {

protected:
    inlineHtml(const inlineHtml& other, pParseContext& C): literalString(other) {}
    
public:
    inlineHtml(const pSourceRange& v): literalString(v, inlineHtmlKind) { }

    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    IMPLEMENT_SUPPORT_MEMBERS(inlineHtml);

};


// NODE: literal null
class literalNull: public literalExpr {

protected:
    literalNull(const literalNull& other, pParseContext& C): literalExpr(other) {}
    
public:
    literalNull(void): literalExpr(literalNullKind) { }

    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    IMPLEMENT_SUPPORT_MEMBERS(literalNull);

};

// literal ID (always represents a class or function symbol name)
//TODO: doesn't inherit by literalExpr?
class literalID: public expr {

    std::string name_;

protected:
    literalID(const literalID& other, pParseContext& C): expr(other), name_(other.name_) {}
    
public:
    literalID(const pSourceRange& name, pParseContext& C):
        expr(literalIDKind),
        name_(name)
    {
    }

    literalID(const namespaceName* name, pParseContext& C):
        expr(literalIDKind),
        name_(name->getFullName())
    {
    }


    pStringRef name(void) const {
        return name_;
    }

    static literalID* create(pStringRef name, pParseContext& C) {
        return new (C) literalID(name, C);
    }

    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    IMPLEMENT_SUPPORT_MEMBERS(literalID);

};

// literal constant (always represents a runtime value symbol. optional target for static or dynamic class)
class literalConstant: public literalExpr {

    std::string name_;
    expr* target_;

protected:
    literalConstant(const literalConstant& other, pParseContext& C): literalExpr(other),
            name_(other.name_), target_(0)
    {
        if(other.target_)
            target_ = other.target_->clone(C);
    }
    
public:
    literalConstant(const pSourceRange& name, pParseContext& C, expr* target = NULL):
        literalExpr(literalConstantKind),
        name_(name),
        target_(target)
    {
    }

    pStringRef name(void) const {
        return name_;
    }

    expr* target(void) const { return target_; }

    stmt::child_iterator child_begin() { return reinterpret_cast<stmt**>(&target_); }
    stmt::child_iterator child_end() { return reinterpret_cast<stmt**>(&target_+1); }

    IMPLEMENT_SUPPORT_MEMBERS(literalConstant);

};

// dynamic ID, i.e. variable variable, or runtime class/method/function name
// Reflection in phc
class dynamicID: public expr {
    
    expr* val_;

protected:
    dynamicID(const dynamicID& other, pParseContext& C): expr(other), val_(0)
    {
        if(other.val_)
            val_ = other.val_->clone(C);
    }
    
public:
    dynamicID(expr* val): expr (dynamicIDKind), val_(val)
    {

    }

    stmt::child_iterator child_begin() { return reinterpret_cast<stmt**>(&val_); }
    stmt::child_iterator child_end() { return reinterpret_cast<stmt**>(&val_+1); }

    expr* val(void) { return val_; }

    IMPLEMENT_SUPPORT_MEMBERS(dynamicID);

};


// NODE: var
class var: public expr {

    enum { TARGET=0, INDICES=1 };

    std::string name_;
    pUInt indirectionCount_; // layers of indirection, i.e. variable variables

    // children_[0] is always target, which may be null. the rest will be array indices
    // numChildren_ is always 1 + number of indices
    stmt** children_;
    pUInt numChildren_;

    // dynamic name based on expression
    expr* dynamicName_;

protected:
    var(const var& other, pParseContext& C): expr(other), name_(other.name_),
            indirectionCount_(other.indirectionCount_), children_(other.children_),
            numChildren_(other.numChildren_)
    {
        deepCopyChildren(children_, other.children_, numChildren_, C);
    }
    
public:
    var(expr* dynamicName, pParseContext& C, expr* target = NULL):
        expr(varKind),
        indirectionCount_(0),
        children_(NULL),
        numChildren_(1),
        dynamicName_(dynamicName)
    {
        children_ = new (C) stmt*[1];
        children_[TARGET] = target;
    }

    var(const pSourceRange& name, pParseContext& C, expr* target = NULL):
        expr(varKind),
        name_(name),
        indirectionCount_(0),
        children_(NULL),
        numChildren_(1),
        dynamicName_(NULL)
    {
        children_ = new (C) stmt*[1];
        children_[TARGET] = target;
    }

    var(const pSourceRange& name, pParseContext& C, expressionList* indices, expr* target = NULL):
        expr(varKind),
        name_(name),
        indirectionCount_(0),
        children_(NULL),
        numChildren_(1+indices->size()),
        dynamicName_(NULL)
    {
        children_ = new (C) stmt*[numChildren_];
        children_[TARGET] = target;
        if (numChildren_ > 1) {
            memcpy(children_+1, &(indices->front()), (numChildren_-1) * sizeof(stmt*));
        }
    }

    bool hasDynamicName(void) const {
        return (dynamicName_ != NULL);
    }

    expr* dynamicName(void) {
        return dynamicName_;
    }

    pStringRef name(void) const {
        if (hasDynamicName())
            return "";
        return name_;
    }

    void setTarget(expr *t) {
        children_[TARGET] = t;
    }

    void setIndirectionCount(pUInt c) { indirectionCount_ = c; }

    expr* target(void) const {
        assert((children_[TARGET] == NULL || isa<expr>(children_[TARGET])) && "unknown object in target");
        return static_cast<expr*>(children_[TARGET]);
    }
    
    /**
     * Whether a variable is simple (just $x) or complex, like $x["a"] or ${"xxx"} 
     */
    bool isSimpleVar() const {
        if(indirectionCount() == 0 && numIndices() == 0 && target() == NULL)
            return true;
        return false;
    }

    pUInt indirectionCount(void) const { return indirectionCount_; }

    stmt::child_iterator child_begin() { return &children_[TARGET]; }
    stmt::child_iterator child_end() { return &children_[TARGET]+numChildren_; }

    pUInt numIndices(void) const { return numChildren_-1; }

    stmt::child_iterator indices_begin() { return &children_[INDICES]; }
    stmt::child_iterator indices_end() { return &children_[INDICES]+(numChildren_-1); }

    IMPLEMENT_SUPPORT_MEMBERS(var);

};

// NODE: assignment
class assignment: public expr {

    enum { LVAL, RVAL, END_EXPR };
    stmt* children_[END_EXPR];
    bool byRef_;

protected:
    assignment(const assignment& other, pParseContext& C): expr(other),
            byRef_(other.byRef_)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[LVAL])
            children_[LVAL] = other.children_[LVAL]->clone(C);
        if(other.children_[RVAL])
            children_[RVAL] = other.children_[RVAL]->clone(C);    }
    
public:
    assignment(expr* lVal, expr* rVal, bool r): expr(assignmentKind), children_(), byRef_(r)
    {
        children_[LVAL] = static_cast<stmt*>(lVal);
        children_[RVAL] = static_cast<stmt*>(rVal);
    }

    expr* lVal(void) { return static_cast<expr*>(children_[LVAL]); }
    expr* rVal(void) { return static_cast<expr*>(children_[RVAL]); }

    bool byRef(void) const { return byRef_; }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    IMPLEMENT_SUPPORT_MEMBERS(assignment);

};

// NODE: list assignment
class listAssignment: public expr {

    enum { VARLIST, RVAL, END_EXPR };
    stmt* children_[END_EXPR];

protected:
    listAssignment(const listAssignment& other, pParseContext& C): expr(other)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[VARLIST])
            children_[VARLIST] = other.children_[VARLIST]->clone(C);
        if(other.children_[RVAL])
            children_[RVAL] = other.children_[RVAL]->clone(C);
    }
    
public:
    listAssignment(block* varList, expr* rVal):
            expr(listAssignmentKind),
            children_()
    {
        children_[VARLIST] = static_cast<stmt*>(varList);
        children_[RVAL] = static_cast<stmt*>(rVal);
    }

    expr* varList(void) { return static_cast<expr*>(children_[VARLIST]); }
    expr* rVal(void) { return static_cast<expr*>(children_[RVAL]); }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    IMPLEMENT_SUPPORT_MEMBERS(listAssignment);

};

// NODE: op assignment
class opAssignment: public expr {

public:
    enum opKind { CONCAT,
                  DIV,
                  MOD,
                  MULT,
                  ADD,
                  SUB,
                  OR,
                  XOR,
                  AND,
                  SHIFT_LEFT,
                  SHIFT_RIGHT
                };

private:
    enum { LVAL, RVAL, END_EXPR };
    stmt* children_[END_EXPR];
    opKind opKind_;

protected:
    opAssignment(const opAssignment& other, pParseContext& C): expr(other),
            opKind_(other.opKind_)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[LVAL])
            children_[LVAL] = other.children_[LVAL]->clone(C);
        if(other.children_[RVAL])
            children_[RVAL] = other.children_[RVAL]->clone(C);
    }
    
public:
    opAssignment(expr* lVal, expr* rVal, opKind op): expr(assignmentKind), children_(), opKind_(op)
    {
        children_[LVAL] = static_cast<stmt*>(lVal);
        children_[RVAL] = static_cast<stmt*>(rVal);
    }

    expr* lVal(void) { return static_cast<expr*>(children_[LVAL]); }
    expr* rVal(void) { return static_cast<expr*>(children_[RVAL]); }

    opKind opKind(void) const { return opKind_; }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    IMPLEMENT_SUPPORT_MEMBERS(opAssignment);

};

// NODE: function/method invoke
class functionInvoke: public expr {

    enum { NAME=0, TARGET=1, INDICES=2 };

    // children_[0] is always expr for name, which may be literalID or dynamicID
    // children_[1] is always target, which may be null. the rest will be array indices
    // numChildren_ is always 1 + number of indices
    stmt** children_;
    pUInt numChildren_;

    // true if this is a constructor invoke, i.e. they used 'new' and the
    // function NAME should resolve to a class name
    bool constructor_;

protected:
    functionInvoke(const functionInvoke& other, pParseContext& C): expr(other),
            children_(0), numChildren_(other.numChildren_)
    {
        deepCopyChildren(children_, other.children_, numChildren_, C);
    }
    
public:
    functionInvoke(expr* name, pParseContext& C):
        expr(functionInvokeKind),
        children_(NULL),
        numChildren_(2),
        constructor_(false)
    {
        children_ = new (C) stmt*[numChildren_];
        children_[NAME] = name;
        children_[TARGET] = NULL;
    }
    functionInvoke(expr* name, pParseContext& C, expressionList* argList, expr* target = NULL):
        expr(functionInvokeKind),
        children_(NULL),
        numChildren_(2+argList->size()),
        constructor_(false)
    {
        children_ = new (C) stmt*[numChildren_];
        children_[NAME] = name;
        children_[TARGET] = target;
        if (numChildren_ > 2) {
            memcpy(children_+2, &(argList->front()), (numChildren_-2) * sizeof(stmt*));
        }
    }
    functionInvoke(expr* name, pParseContext& C, expr* arg1, expr* target = NULL):
        expr(functionInvokeKind),
        children_(NULL),
        numChildren_(3),
        constructor_(false)
    {
        children_ = new (C) stmt*[numChildren_];
        children_[NAME] = name;
        children_[TARGET] = target;
        memcpy(children_+2, &arg1, sizeof(stmt*));
    }

    void setConstructor(void) { constructor_ = true; }

    bool constructor(void) const { return constructor_; }

    bool hasLiteralName(void) const { return isa<literalID>(children_[NAME]); }

    bool hasLiteralTarget(void) const {
        return (children_[TARGET] != NULL && isa<literalID>(children_[TARGET]));
    }

    void setTarget(expr *t) {
        children_[TARGET] = t;
    }

    expr* name(void) {
        assert(isa<literalID>(children_[NAME]) || isa<dynamicID>(children_[NAME]));
        return static_cast<expr*>(children_[NAME]);
    }

    // for when hasLiteralName()
    pStringRef literalName(void) {
        assert(isa<literalID>(children_[NAME]) && "liternalName expected literalID in NAME");
        literalID *n = cast<literalID>(children_[NAME]);
        return n->name();
    }

    // for when hasLiteralTarget()
    pStringRef literalTargetName(void) {
        assert(isa<literalID>(children_[TARGET]) && "liternalTargetName expected literalID in TARGET");
        literalID *n = cast<literalID>(children_[TARGET]);
        return n->name();
    }

    expr* target(void) {
        assert((children_[TARGET] == NULL || isa<expr>(children_[TARGET])) && "unknown object in target");
        return static_cast<expr*>(children_[TARGET]);
    }

    pUInt numArgs(void) const { return numChildren_-2; }

    stmt::child_iterator child_begin() { return &children_[0]; }
    stmt::child_iterator child_end() { return &children_[0]+numChildren_; }

    stmt::child_iterator args_begin() { return &children_[INDICES]; }
    stmt::child_iterator args_end() { return &children_[INDICES]+(numChildren_-INDICES); }

    stmt::child_range args() { return stmt::child_range(args_begin(), args_end()); }

    IMPLEMENT_SUPPORT_MEMBERS(functionInvoke);

};

// NOP statement such as ;;
class emptyStmt: public stmt {

protected:
    emptyStmt(const emptyStmt& other, pParseContext& C): stmt(other) {}
    
public:
	emptyStmt() : stmt(emptyStmtKind) {}


    stmt::child_iterator child_begin() { return child_iterator(); }
    stmt::child_iterator child_end() { return child_iterator(); }

    IMPLEMENT_SUPPORT_MEMBERS(emptyStmt);

};

// NODE: unary operator
class unaryOp: public expr {

public:
    enum opKind { NEGATIVE, POSITIVE, LOGICALNOT, BITWISENOT };

private:
    expr* rVal_;
    opKind opKind_;

protected:
    unaryOp(const unaryOp& other, pParseContext& C): expr(other), rVal_(0),
            opKind_(other.opKind_)
    {
        if(other.rVal_)
            rVal_ = other.rVal_->clone(C);
    }
    
public:

    unaryOp(expr* rVal, opKind k): expr(unaryOpKind), rVal_(rVal), opKind_(k) {
        if (opKind_ == NEGATIVE) {
            // if our expression is a simple literal int, flag its sign
            if (literalInt* i = dyn_cast<literalInt>(rVal))
                i->setNegative(true);
        }
    }

    expr* rVal(void) { return rVal_; }

    stmt::child_iterator child_begin() { return reinterpret_cast<stmt**>(&rVal_); }
    stmt::child_iterator child_end() { return reinterpret_cast<stmt**>(&rVal_+1); }

    opKind opKind(void) const { return opKind_; }

    IMPLEMENT_SUPPORT_MEMBERS(unaryOp);

};

// NODE: pre operator
class preOp: public expr {

public:
    enum opKind { INC, DEC };

private:
    expr* rVal_;
    opKind opKind_;

protected:
    preOp(const preOp& other, pParseContext& C): expr(other), rVal_(0),
            opKind_(other.opKind_)
    {
        if(other.rVal_)
            rVal_ = other.rVal_->clone(C);
    }
    
public:

    preOp(expr* rVal, opKind k): expr(preOpKind), rVal_(rVal), opKind_(k)
    {
    }

    expr* rVal(void) { return rVal_; }

    stmt::child_iterator child_begin() { return reinterpret_cast<stmt**>(&rVal_); }
    stmt::child_iterator child_end() { return reinterpret_cast<stmt**>(&rVal_+1); }

    opKind opKind(void) const { return opKind_; }

    IMPLEMENT_SUPPORT_MEMBERS(preOp);

};

// NODE: post operator
class postOp: public expr {

public:
    enum opKind { INC, DEC };

private:
    expr* rVal_;
    opKind opKind_;

protected:
    postOp(const postOp& other, pParseContext& C): expr(other), rVal_(0),
            opKind_(other.opKind_)
    {
        if(other.rVal_)
            rVal_ = other.rVal_->clone(C);
    }
    
public:

    postOp(expr* rVal, opKind k): expr(postOpKind), rVal_(rVal), opKind_(k)
    {
    }

    expr* rVal(void) { return rVal_; }

    stmt::child_iterator child_begin() { return reinterpret_cast<stmt**>(&rVal_); }
    stmt::child_iterator child_end() { return reinterpret_cast<stmt**>(&rVal_+1); }

    opKind opKind(void) const { return opKind_; }

    IMPLEMENT_SUPPORT_MEMBERS(postOp);

};


// NODE: binary operator
class binaryOp: public expr {

public:
    enum opKind { CONCAT,
                  BOOLEAN_AND,
                  BOOLEAN_OR,
                  BOOLEAN_XOR,
                  DIV,
                  MOD,
                  MULT,
                  ADD,
                  SUB,
                  GREATER_THAN,
                  LESS_THAN,
                  GREATER_OR_EQUAL,
                  LESS_OR_EQUAL,
                  EQUAL,
                  NOT_EQUAL,
                  IDENTICAL,
                  NOT_IDENTICAL,
                  BIT_OR,
                  BIT_AND,
                  BIT_XOR,
                  INSTANCEOF,
                  SHIFT_LEFT,
                  SHIFT_RIGHT
              };

private:
    enum { LVAL, RVAL, END_EXPR };
    stmt* children_[END_EXPR];
    opKind opKind_;

protected:
    binaryOp(const binaryOp& other, pParseContext& C): expr(other), opKind_(other.opKind_)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[LVAL])
            children_[LVAL] = other.children_[LVAL]->clone(C);
        if(other.children_[RVAL])
            children_[RVAL] = other.children_[RVAL]->clone(C);
    }
    
public:

    binaryOp(expr* lVal, expr* rVal, opKind k): expr(binaryOpKind), children_(), opKind_(k)
    {
        children_[LVAL] = static_cast<stmt*>(lVal);
        children_[RVAL] = static_cast<stmt*>(rVal);
    }

    expr* lVal(void) { return static_cast<expr*>(children_[LVAL]); }
    expr* rVal(void) { return static_cast<expr*>(children_[RVAL]); }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    IMPLEMENT_SUPPORT_MEMBERS(binaryOp);

    opKind opKind(void) const { return opKind_; }

};

// ternary shorthand for ifs ?:
class conditionalExpr: public expr {

private:
    enum { COND, TRUEEXPR, FALSEEXPR, END_EXPR };
    stmt* children_[END_EXPR];

protected:
    conditionalExpr(const conditionalExpr& other, pParseContext& C): expr(other)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[COND])
            children_[COND] = other.children_[COND]->clone(C);
        if(other.children_[TRUEEXPR])
            children_[TRUEEXPR] = other.children_[TRUEEXPR]->clone(C);
        if(other.children_[FALSEEXPR])
            children_[FALSEEXPR] = other.children_[FALSEEXPR]->clone(C);
    }
    
public:

    conditionalExpr(expr* condition, expr* trueexpr, expr* falseexpr): expr(conditionalExprKind), children_()
    {
        children_[COND] = static_cast<stmt*>(condition);
        children_[TRUEEXPR] = static_cast<stmt*>(trueexpr);
        children_[FALSEEXPR] = static_cast<stmt*>(falseexpr);
    }

    expr* condition(void) const{ return static_cast<expr*>(children_[COND]); }
    expr* trueExpr(void) const{ return static_cast<expr*>(children_[TRUEEXPR]); }
    expr* falseExpr(void) const{ return static_cast<expr*>(children_[FALSEEXPR]); }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    IMPLEMENT_SUPPORT_MEMBERS(conditionalExpr);

};

class lambda: public expr {

    enum { SIG, BODY, END_EXPR };
    stmt* children_[END_EXPR];

protected:
    lambda(const lambda& other, pParseContext& C): expr(other)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[SIG])
            children_[SIG] = other.children_[SIG]->clone(C);
        if(other.children_[BODY])
            children_[BODY] = other.children_[BODY]->clone(C);
    }

public:
    lambda(signature* sig, block* body):
        expr(lambdaKind),
        children_()
    {
        children_[SIG] = sig;
        children_[BODY] = body;
    }

    signature* sig(void) { return static_cast<signature*>(children_[SIG]); }
    block* body(void) { return static_cast<block*>(children_[BODY]); }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }

    IMPLEMENT_SUPPORT_MEMBERS(lambda);

};


// This needs to be after the class label.
// function declaration
class functionDecl: public decl {

    enum { SIG, BODY, END_EXPR };
    stmt* children_[END_EXPR];
    // This contains the id the next label will recieve, so a function has labelCount_-1 labels.
    pUInt labelCount_;
    
protected:
    functionDecl(const functionDecl& other, pParseContext& C): decl(other),
            labelCount_(other.labelCount_)
    {
        memset(children_, 0, sizeof(children_));
        if(other.children_[SIG])
            children_[SIG] = other.children_[SIG]->clone(C);
        if(other.children_[BODY])
            children_[BODY] = other.children_[BODY]->clone(C);
    }

public:
    functionDecl(signature* sig, block* body):
        decl(functionDeclKind),
        children_(), labelCount_(0)
    {
        children_[SIG] = sig;
        children_[BODY] = body;
    }

    signature* sig(void) { return static_cast<signature*>(children_[SIG]); }
    block* body(void) { return static_cast<block*>(children_[BODY]); }

    stmt::child_iterator child_begin() { return (stmt**)&children_[0]; }
    stmt::child_iterator child_end() { return (stmt**)&children_[0]+END_EXPR; }
    
    IMPLEMENT_SUPPORT_MEMBERS(functionDecl);

};


} } // namespace

#endif /* COR_PAST_H_ */
