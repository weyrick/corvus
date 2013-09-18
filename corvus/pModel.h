/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PMODEL_H_
#define COR_PMODEL_H_

#include "corvus/pTypes.h"
#include "pDB.h"

//#include <sqlite3.h>
#include <map>
#include <vector>

#include <iostream>

struct sqlite3;

namespace corvus {

namespace model {


class mFunction: public db::dbRow { };

class mClass: public db::dbRow { };

class mClassDecl: public db::dbRow { };

class mConstant: public db::dbRow { };

struct mMultipleDecl {
    typedef std::pair<db::pDB::oid, pSourceRange> locData;
    std::string symbol;
    std::string realPath;
    std::vector<locData> redecl_locs;
};

} // end model namespace

class pModel {
public:

    typedef db::pDB::oid oid;
    typedef db::pDB::RowList RowList;
    //typedef std::vector<model::mFunction> FunctionList;
    //typedef std::vector<model::mClass> ClassList;
    //typedef std::vector<model::mClassDecl> ClassDeclList;
    //typedef std::vector<model::mConstant> ConstantList;
    typedef db::pDB::RowList FunctionList;
    typedef db::pDB::RowList ClassList;
    typedef db::pDB::RowList ClassDeclList;
    typedef db::pDB::RowList ConstantList;
    typedef db::pDB::RowList UndeclList;
    typedef db::pDB::RowList UnusedList;
    typedef std::vector<model::mMultipleDecl> MultipleDeclList;

    typedef std::map<std::string, oid> IDMap;

    // general
    enum {
    MULTIPLE_IDS = -1,
    NULLID   = 0,
    NO_FLAGS = 0,

    // constants
    CONST       = 0,
    DEFINE      = 1,

    // flags
    STATIC   = 0x1,
    ABSTRACT = 0x2,
    FINAL    = 0x4,

    // function types
    TOP_LEVEL = 0,
    FUNCTION  = 1,
    METHOD    = 2,

    // function var types
    PARAM     = 0,
    FREE_VAR  = 1,

    // visibility
    PUBLIC    = 0,
    PROTECTED = 1,
    PRIVATE   = 2,

    // data types
    TYPE_UNKNOWN = 0,
    TYPE_NULL = 1,
    // array, str, bool, etc

    // class types
    CLASS   = 0,
    IFACE   = 1,

    // class decl types
    // share CONST = 0 above
    PROPERTY = 2,

    // class relation types
    EXTENDS = 0,
    IMPLEMENTS = 1

    };

private:

    db::pDB *db_;

    IDMap modules_;
    mutable IDMap namespaces_;

    void makeTables();

public:

    pModel(sqlite3 *db, bool trace=false): db_(0) {
        db_ = new db::pDB(db, trace);
        makeTables();
    }

    ~pModel() {
        delete db_;
    }

    void setTrace(bool trace) { if (db_) db_->setTrace(trace); }

    void commit() {
        if (db_) db_->commit();
    }

    void begin() {
        if (db_) db_->begin();
    }

    // DEFINE, MUTATE
    oid getSourceModuleOID(pStringRef realPath, pStringRef hash="", bool deleteFirst=false);
    oid defineClass(oid ns_id, oid m_id, pStringRef name, int type, int extends_count, int implements_count,
                    pStringRef extends, pStringRef implements, pSourceRange range);
    void defineClassDecl(oid c_id, pStringRef name, int type, int flags, int vis, pStringRef defaultVal, pSourceRange range);
    void defineClassRelation(oid lhs_c_id, int type, oid rhs_c_id);
    oid defineFunction(oid ns_id, oid m_id, oid c_id, pStringRef name,
                        int type, int flags, int vis, int minA, int maxA, pSourceRange range);
    void defineFunctionVar(oid f_id, pStringRef name,
                          int type, int flags, int datatype, pStringRef datatype_obj,
                          pStringRef defaultVal,
                          pSourceRange range);
    void defineFunctionVarUse(oid f_id, pStringRef name, pSourceRange range);

    void defineConstant(oid m_id, pStringRef name, int type, pStringRef val, pSourceRange range);

    void resolveClassRelations();
    void resolveMultipleDecls(oid m_id);
    void refreshClassModel(pStringRef graphFileName="");

    // QUERY
    bool sourceModuleDirty(pStringRef realPath, pStringRef hash) const;
    oid getNamespaceOID(pStringRef ns, bool create=false) const;
    std::string getNamespaceName(oid ns_id) const;
    oid getRootNamespaceOID() const {
        return getNamespaceOID("\\", true);
    }

    ConstantList queryConstants(pStringRef name) const;
    ClassList queryClasses(oid ns_id, pStringRef name, oid m_id = pModel::NULLID) const;
    ClassDeclList queryClassDecls(oid c_id, pStringRef name) const;
    ClassDeclList queryClassDecls(std::vector<oid> c_id_list, pStringRef name) const;
    FunctionList queryFunctions(oid ns_id, oid c_id, pStringRef name) const;

    std::pair<oid, std::string> resolveFQN(oid ns_id, pStringRef name) const;

    oid lookupClass(oid ns_id, pStringRef name, oid m_id = pModel::NULLID) const;
    oid lookupFunction(oid ns_id, oid c_id, pStringRef name) const;

    ClassList getUnresolvedClasses() const;
    MultipleDeclList getMultipleDecls(oid m_id = pModel::NULLID) const;
    UndeclList getUndeclaredUses(oid m_id = pModel::NULLID) const;
    UnusedList getUnusedDecls(oid m_id = pModel::NULLID) const;


};

} // namespace

#endif
