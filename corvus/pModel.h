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

#include <sqlite3.h>
#include <map>
#include <vector>

#include <iostream>

namespace corvus {

// these are for caching data retrieved from sqlite
namespace model {

// a simple wrapper for a row in the model db
// which saves us from having to make structs for each row
// here we can just access it by field name
class dbRow {
public:
    typedef std::map<const std::string, std::string> StringMap;
    typedef std::map<const std::string, long> IntMap;
protected:
    StringMap fields_;
    mutable IntMap intFields_; // lazyily built cache
public:
    const std::string& get(pStringRef key) const {
        //std::cout << "key: " << key.str() << std::endl;
        StringMap::const_iterator i = fields_.find(key);
        assert(i != fields_.end() && "key not found");
        return i->second;
    }
    int getAsInt(pStringRef key) const;
    sqlite3_int64 getAsOID(pStringRef key) const;
    sqlite3_int64 getID() const {
        return getAsOID("id");
    }
    void set(pStringRef key, pStringRef val) {
        fields_[key] = val;
    }
};

class mFunction: public dbRow { };

class mClass: public dbRow { };

class mClassDecl: public dbRow { };

class mConstant: public dbRow { };

} // end model namespace

class pModel {
public:

    typedef sqlite3_int64 oid;
    typedef std::vector<model::mFunction> FunctionList;
    typedef std::vector<model::mClass> ClassList;
    typedef std::vector<model::mClassDecl> ClassDeclList;
    typedef std::vector<model::mConstant> ConstantList;
    typedef std::map<std::string, oid> IDMap;

    // general
    enum {
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
    T_UNKNOWN = 0,
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

    sqlite3 *db_;
    bool trace_;

    IDMap modules_;
    mutable IDMap namespaces_;

    void sql_execute(pStringRef query) const;
    oid sql_insert(pStringRef query) const;
    oid sql_select_single_id(pStringRef query) const;
    void sql_setup();
    void sql_done();

    void makeTables();

    std::string oidOrNull(oid val);
    std::string sql_string(pStringRef val, bool allowNull=true);

    template <typename LTYPE>
    void list_query(pStringRef query, LTYPE &result) const;

public:

    pModel(sqlite3 *db, bool trace=false): db_(db), trace_(trace) {
        sql_setup();
    }

    void commit(bool begin=true);

    bool sourceModuleDirty(pStringRef realPath, pStringRef hash);
    oid getSourceModuleOID(pStringRef realPath, pStringRef hash="", bool deleteFirst=false);
    oid getNamespaceOID(pStringRef ns, bool create=false) const;
    oid getRootNamespaceOID() const {
        return getNamespaceOID("\\", true);
    }

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

    void defineConstant(oid m_id, pStringRef name, int type, pStringRef val, pSourceRange range);

    ClassList getUnresolvedClasses() const;
    void resolveClassRelations();

    ConstantList queryConstants(pStringRef name) const;
    ClassList queryClasses(oid ns_id, pStringRef name, oid m_id = pModel::NULLID) const;
    ClassDeclList queryClassDecls(oid c_id, pStringRef name) const;
    FunctionList queryFunctions(oid ns_id, oid c_id, pStringRef name) const;

    oid lookupClass(oid ns_id, pStringRef name, oid m_id = pModel::NULLID) const;

};

} // namespace

#endif /* COR_PSOURCEFILE_H_ */
