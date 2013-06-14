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
        assert(fields_.find(key) != fields_.end() && "key not found");
        StringMap::const_iterator i = fields_.find(key);
        return i->first;
    }
    int getAsInt(pStringRef key) const;
    void set(pStringRef key, pStringRef val) {
        fields_[key] = val;
    }
};

class mFunction: public dbRow {

};

class mClass: public dbRow {

};

} // end model namespace

class pModel {
public:

    typedef sqlite3_int64 oid;
    typedef std::vector<model::mFunction> FunctionList;
    typedef std::vector<model::mClass> ClassList;
    typedef std::map<std::string, oid> IDMap;

    // general
    const static int NULLID   = 0;
    const static int NO_FLAGS = 0;

    // constants
    const static int CLASS_CONST = 0;
    const static int DEFINE      = 1;

    // flags
    const static int STATIC   = 0x1;
    const static int ABSTRACT = 0x2;

    // function types
    const static int TOP_LEVEL = 0;
    const static int FUNCTION  = 1;
    const static int METHOD    = 2;

    // function var types
    const static int PARAM     = 0;
    const static int FREE_VAR  = 1;

    // visibility
    const static int PUBLIC    = 0;
    const static int PROTECTED = 1;
    const static int PRIVATE   = 2;

    // data types
    const static int T_UNKNOWN = 0;
    // array, str, bool, etc

    // class types
    const static int CLASS   = 0;
    const static int IFACE   = 1;

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

    pStringRef oidOrNull(oid val);
    pStringRef strOrNull(pStringRef val);

    template <typename LTYPE>
    void list_query(pStringRef query, LTYPE &result) const;

public:

    pModel(sqlite3 *db, bool trace=false): db_(db), trace_(trace) {
        sql_setup();
    }

    void commit(bool begin=true);

    bool sourceModuleDirty(pStringRef realPath, pStringRef hash);
    oid getSourceModuleOID(pStringRef realPath, pStringRef hash, bool deleteFirst);
    oid getNamespaceOID(pStringRef ns, bool create=false) const;
    oid getRootNamespaceOID() const {
        return getNamespaceOID("\\", true);
    }

    oid defineClass(oid ns_id, oid m_id, pStringRef name, pSourceRange range);
    oid defineFunction(oid ns_id, oid m_id, oid c_id, pStringRef name,
                        int type, int flags, int vis, int minA, int maxA, pSourceRange range);
    void defineFunctionVar(oid f_id, pStringRef name,
                          int type, int flags, int datatype, pStringRef datatype_obj,
                          pStringRef defaultVal,
                          int sl, int sc);

    void defineConstant(oid m_id, int type, pStringRef name, pStringRef val, pSourceRange range);

    ClassList queryClasses(oid ns_id, pStringRef name) const;
    FunctionList queryFunctions(oid ns_id, oid c_id, pStringRef name) const;

};

} // namespace

#endif /* COR_PSOURCEFILE_H_ */
