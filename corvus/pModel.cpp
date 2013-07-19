/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/pModel.h"

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>

namespace corvus { 

namespace model {
int dbRow::getAsInt(pStringRef key) const {
    if (intFields_.find(key) != intFields_.end())
        return intFields_[key];
    assert(fields_.find(key) != fields_.end() && "getAsInt key not found");
    // convert, cache
    StringMap::const_iterator v = fields_.find(key);
    intFields_[key] = atol(v->second.c_str());
    return intFields_[key];
}

sqlite3_int64 dbRow::getAsOID(pStringRef key) const {
    assert(fields_.find(key) != fields_.end() && "getAsInt key not found");
    StringMap::const_iterator v = fields_.find(key);
    return atoll(v->second.c_str());
}

}

void pModel::sql_execute(pStringRef query) const {
    char *errMsg;
    int rc = sqlite3_exec(db_, query.begin(), NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << query.str()
                  << "\n" << errMsg << "\n";
        sqlite3_free(errMsg);
        exit(1);
    }
    else if (trace_) {
        std::cerr << "TRACE: " << query.str() << std::endl;
    }
}

pModel::oid pModel::sql_insert(pStringRef query) const {
    sql_execute(query);
    oid r = sqlite3_last_insert_rowid(db_);
    if (trace_) {
        std::cerr << "TRACE: insert id #" << r << std::endl;
    }
    return r;
}

pModel::oid pModel::sql_select_single_id(pStringRef query) const {

    sqlite3_stmt *stmt;
    pModel::oid result = pModel::NULLID;

    int rc = sqlite3_prepare_v2(db_, query.str().c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << query.str() << "\n";
        std::cerr << sqlite3_errmsg(db_) << "\n";
        exit(1);
    }
    else if (trace_) {
        std::cerr << "TRACE: " << query.str() << std::endl;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        result = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return result;

}

void pModel::sql_setup() {
    sql_execute("PRAGMA foreign_keys = ON");
    sql_execute("BEGIN");
    makeTables();
    commit();
}

void pModel::commit(bool begin) {
    sql_execute("COMMIT");
    if (begin)
        sql_execute("BEGIN");
}

void pModel::makeTables() {

    const char *SM = "CREATE TABLE IF NOT EXISTS sourceModule (" \
                         "id INTEGER PRIMARY KEY,"
                         "realpath TEXT UNIQUE NOT NULL," \
                         "hash TEXT" \
                         ")";
    sql_execute(SM);

    // type:
    //   0 - const (outside class)
    //   1 - define()
    const char *SD = "CREATE TABLE IF NOT EXISTS constant (" \
                         "id INTEGER PRIMARY KEY,"
                         "sourceModule_id INTEGER NOT NULL," \
                         "type INTEGER NOT NULL," \
                         "name TEXT NOT NULL," \
                         "value TEXT NOT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(sourceModule_id) REFERENCES sourceModule(id) ON DELETE CASCADE"
                         ")";
    sql_execute(SD);

    const char *SU = "CREATE TABLE IF NOT EXISTS constant_use (" \
                         "id INTEGER PRIMARY KEY,"
                         "constant_id INTEGER NOT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                             "FOREIGN KEY(constant_id) REFERENCES constant(id) ON DELETE CASCADE"
                         ")";
    sql_execute(SU);

    const char *NS = "CREATE TABLE IF NOT EXISTS namespace (" \
                         "id INTEGER PRIMARY KEY,"
                         "namespace TEXT UNIQUE NOT NULL" \
                         ")";
    sql_execute(NS);

    // type:
    //   0 - class
    //   1 - interface
    // flags:
    //   0x1 ABSTRACT
    const char *CL = "CREATE TABLE IF NOT EXISTS class (" \
                         "id INTEGER PRIMARY KEY,"
                         "sourceModule_id INTEGER NOT NULL," \
                         "namespace_id INTEGER NULL," \
                         "name TEXT NOT NULL,"
                         "type INTEGER NOT NULL," \
                         "flags INTEGER NOT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "end_line INTEGER NOT NULL," \
                         "end_col INTEGER NOT NULL," \
                         // text versions
                         "extends TEXT NULL," \
                         "implements TEXT NULL," \
                         //
                         "FOREIGN KEY(namespace_id) REFERENCES namespace(id) ON DELETE CASCADE," \
                         "FOREIGN KEY(sourceModule_id) REFERENCES sourceModule(id) ON DELETE CASCADE"
                         ")";
    sql_execute(CL);

    // the relations go "lhs TYPE rhs"
    // type:
    //   0 - extends
    //   1 - implements
    const char *CR = "CREATE TABLE IF NOT EXISTS class_relations (" \
            "id INTEGER PRIMARY KEY,"
            "lhs_class_id INTEGER NOT NULL," \
            "type INTEGER NOT NULL," \
            "rhs_class_id INTEGER NOT NULL," \
            "FOREIGN KEY(lhs_class_id) REFERENCES class(id) ON DELETE CASCADE," \
            "FOREIGN KEY(rhs_class_id) REFERENCES class(id) ON DELETE CASCADE" \
                         ")";
    sql_execute(CR);


    // type:
    //   0 - class property
    //   1 - class constant
    // flags:
    //   0x1 STATIC
    //   0x2 ABSTRACT
    // visibility:
    //   2 PRIVATE
    //   1 PROTECTED
    //   0 PUBLIC
    const char *CD = "CREATE TABLE IF NOT EXISTS class_decl (" \
                         "id INTEGER PRIMARY KEY,"
                         "class_id INTEGER NOT NULL," \
                         "name TEXT NULL," \
                         "type INTEGER NOT NULL," \
                         "flags INTEGER NOT NULL," \
                         "visibility INTEGER NOT NULL," \
                         "defaultVal TEXT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(class_id) REFERENCES class(id) ON DELETE CASCADE"
                         ")";
    sql_execute(CD);

    const char *CU = "CREATE TABLE IF NOT EXISTS class_decl_use (" \
                         "id INTEGER PRIMARY KEY,"
                         "class_id INTEGER NOT NULL," \
                         // if using an undeclared, this will be NULL
                         "class_decl_id INTEGER NULL," \
                         // if using an undeclared, this will contain the name
                         "name TEXT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(class_id) REFERENCES class(id) ON DELETE CASCADE," \
                         "FOREIGN KEY(class_decl_id) REFERENCES class_decl(id) ON DELETE CASCADE"
                         ")";
    sql_execute(CU);

    // type:
    //   0 - top level main
    //   1 - function
    //   2 - method
    // flags:
    //   0x1 STATIC
    //   0x2 ABSTRACT
    // visibility:
    //   2 PRIVATE
    //   1 PROTECTED
    //   0 PUBLIC
    const char *FN = "CREATE TABLE IF NOT EXISTS function (" \
                         "id INTEGER PRIMARY KEY,"
                         "sourceModule_id INTEGER NOT NULL," \
                         "namespace_id INTEGER NOT NULL," \
                         "class_id INTEGER NULL," \
                         "name TEXT,"
                         "type INTEGER NOT NULL," \
                         "flags INTEGER NOT NULL," \
                         "visibility INTEGER NOT NULL," \
                         "minArity INTEGER NOT NULL," \
                         "maxArity INTEGER NOT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "end_line INTEGER NOT NULL," \
                         "end_col INTEGER NOT NULL," \
                         "FOREIGN KEY(namespace_id) REFERENCES namespace(id) ON DELETE CASCADE," \
                         "FOREIGN KEY(class_id) REFERENCES class(id) ON DELETE CASCADE," \
                         "FOREIGN KEY(sourceModule_id) REFERENCES sourceModule(id) ON DELETE CASCADE"
                         ")";
    sql_execute(FN);

    // type:
    //   0 - parameter
    //   1 - free variable
    // datatype:
    //   0 - unknown
    //   1 - integer
    //   2 - string
    //   3 - array
    //   4 - bool
    //   5 - null
    //   6 - object
    // flags:
    //   0x1 STATIC
    const char *FV = "CREATE TABLE IF NOT EXISTS function_var (" \
                         "id INTEGER PRIMARY KEY,"
                         "function_id INTEGER NOT NULL," \
                         "name TEXT,"
                         "type INTEGER NOT NULL," \
                         "flags INTEGER NOT NULL," \
                         "datatype INTEGER NOT NULL," \
                         "datatype_obj TEXT NULL," \
                         "defaultVal TEXT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(function_id) REFERENCES function(id) ON DELETE CASCADE"
                         ")";
    sql_execute(FV);


    const char *FVU = "CREATE TABLE IF NOT EXISTS function_var_use (" \
                         "id INTEGER PRIMARY KEY,"
                         "function_id INTEGER NOT NULL," \
                         // if using an undeclared, this will be NULL
                         "function_var_id INTEGER NULL," \
                         // if using an undeclared, this will contain the name
                         "name TEXT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(function_id) REFERENCES function(id) ON DELETE CASCADE," \
                         "FOREIGN KEY(function_var_id) REFERENCES function_var(id) ON DELETE CASCADE"
                         ")";
    sql_execute(FVU);


    const char *FU = "CREATE TABLE IF NOT EXISTS function_use (" \
                         "id INTEGER PRIMARY KEY,"
                         // if using an undeclared, this will be NULL
                         "function_id INTEGER NULL," \
                         // if using an undeclared, this will contain the name
                         "name TEXT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(function_id) REFERENCES function(id) ON DELETE CASCADE" \
                         ")";
    sql_execute(FU);

}

bool pModel::sourceModuleDirty(pStringRef realPath, pStringRef hash) {

    std::stringstream sql;

    sql << "SELECT oid, hash FROM sourceModule WHERE realPath='" << realPath.str() << "'";

    sqlite3_stmt *stmt;
    pModel::oid existing = pModel::NULLID;
    std::string existing_hash;

    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << sql.str() << "\n";
        std::cerr << sqlite3_errmsg(db_) << "\n";
        exit(1);
    }
    else if (trace_) {
        std::cerr << "TRACE: " << sql.str() << std::endl;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        existing = sqlite3_column_int(stmt, 0);
        existing_hash = (char*)sqlite3_column_text(stmt, 1);
    }

    sqlite3_finalize(stmt);

    if (existing != pModel::NULLID) {
        // not in our local cache but it's in the db model
        // we have it in the db. check hash to see if we should remodel it
        return (pStringRef(hash) != existing_hash);
    }
    else {
        // not in model, it's dirty
        return true;
    }

}

pModel::oid pModel::getSourceModuleOID(pStringRef realPath, pStringRef hash, bool deleteFirst) {

    if (modules_.find(realPath) != modules_.end()) {
        return modules_[realPath];
    }    

    std::stringstream sql;

    if (!deleteFirst) {
        sql << "SELECT oid FROM sourceModule WHERE realPath='" << realPath.str() << "'";

        pModel::oid existing = sql_select_single_id(sql.str());
        if (existing != pModel::NULLID) {
            modules_[realPath] = existing;
            return existing;
        }
    }
    else {
        sql << "DELETE FROM sourceModule WHERE realPath='" << realPath.str() << "'";
        sql_execute(sql.str());
    }
    sql.str("");

    sql << "INSERT INTO sourceModule VALUES (NULL, '" << realPath.str() << "', '" << hash.str() << "')";
    oid result = sql_insert(sql.str().c_str());
    modules_[realPath] = result;
    return result;

}

pModel::oid pModel::getNamespaceOID(pStringRef ns, bool create) const {

    if (namespaces_.find(ns) != namespaces_.end()) {
        return namespaces_[ns];
    }

    std::stringstream sql;

    sql << "SELECT oid FROM namespace WHERE namespace='" << ns.str() << "'";

    pModel::oid existing = sql_select_single_id(sql.str());
    if (existing != pModel::NULLID) {
        namespaces_[ns] = existing;
        return existing;
    }

    if (!create)
        return pModel::NULLID;

    sql.str("");

    sql << "INSERT INTO namespace VALUES (NULL, '" << ns.str() << "')";
    oid result = sql_insert(sql.str().c_str());
    namespaces_[ns] = result;
    return result;

}

pModel::oid pModel::defineClass(pModel::oid ns_id, pModel::oid m_id, pStringRef name, pSourceRange range) {

    int type = pModel::CLASS;
    int flags = pModel::NO_FLAGS;

    std::stringstream sql;
    sql << "INSERT INTO class VALUES (NULL,"
        << m_id << ','
        << ns_id
        << ",'" << name.str() << "',"
        << type << ','
        << flags << ','
        << range.startLine  << ',' << range.startCol  << ',' << range.endLine  << ',' << range.endCol
        << ",'',''" // extends, implements
        << ")";
    return sql_insert(sql.str().c_str());

}

std::string pModel::oidOrNull(oid val) {
    if (val) {
        std::stringstream conv;
        conv << val;
        return conv.str();
    }
    else {
        return "NULL";
    }
}

// handles escape quotes
std::string pModel::sql_string(pStringRef val, bool allowNull) {
    if (!val.empty()) {
        std::string conv;
        int quotes = val.count('\'');
        // get room for doubling up the quotes
        conv.reserve(val.size()+quotes+2); // +2 is outer quotes
        conv.push_back('\'');
        for (pStringRef::iterator i = val.begin();
             i != val.end();
             ++i) {
            if (*i == '\'') {
                conv.push_back('\'');
                conv.push_back('\'');
            }
            else {
                conv.push_back(*i);
            }
        }
        conv.push_back('\'');
        return conv;
    }
    else {
        if (allowNull)
            return "NULL";
        else
            return "''";
    }
}

pModel::oid pModel::defineFunction(oid ns_id, oid m_id, oid c_id, pStringRef name,
                    int type, int flags, int vis, int minA, int maxA, pSourceRange range) {

    std::stringstream sql;
    sql << "INSERT INTO function VALUES (NULL,"
        << m_id << ','
        << ns_id << ','
        << oidOrNull(c_id)
        << ",'" << name.str() << "',"
        << type << ','
        << flags << ','
        << vis << ','
        << minA << ','
        << maxA << ','
        << range.startLine  << ',' << range.startCol  << ',' << range.endLine  << ',' << range.endCol
        << ")";
    return sql_insert(sql.str().c_str());

}

void pModel::defineClassDecl(oid c_id, pStringRef name, int type, int flags, int vis, pStringRef defaultVal, pSourceRange range) {

    std::stringstream sql;
    sql << "INSERT INTO class_decl VALUES (NULL,"
        << c_id
        << ",'" << name.str() << "',"
        << type << ','
        << flags << ','
        << vis << ','
        << sql_string(defaultVal) << ","
        << range.startLine  << ',' << range.startCol
        << ")";

    sql_insert(sql.str().c_str());

}

void pModel::defineFunctionVar(oid f_id, pStringRef name,
                    int type, int flags, int datatype, pStringRef datatype_obj,
                    pStringRef defaultVal,
                    pSourceRange range) {

    std::stringstream sql;

    sql << "INSERT INTO function_var VALUES (NULL,"
        << f_id << ','
        << "'" << name.str() << "'" << ','
        << type << ','
        << flags << ','
        << datatype << ','
        << sql_string(datatype_obj) << ","
        << sql_string(defaultVal) << ","
        << range.startLine  << ',' << range.startCol
        << ")";
    sql_insert(sql.str().c_str());

}

void pModel::defineConstant(oid m_id, pStringRef name, int type, pStringRef val, pSourceRange range) {

    std::stringstream sql;

    sql << "INSERT INTO constant VALUES (NULL,"
        << m_id << ','
        << type << ','
        << "'" << name.str() << "'" << ','
        << sql_string(val,false) << ','
        << range.startLine  << ',' << range.startCol
        << ")";
    sql_insert(sql.str().c_str());

}

template <typename LTYPE>
void pModel::list_query(pStringRef query, LTYPE &result) const {
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_, query.str().c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << query.str() << "\n";
        std::cerr << sqlite3_errmsg(db_) << "\n";
        exit(1);
    }
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        typename LTYPE::value_type f;
        for (int i = 0; i < sqlite3_column_count(stmt); i++) {
            pStringRef key = sqlite3_column_name(stmt, i);
            pStringRef val;
            char *cVal = (char*)sqlite3_column_text(stmt, i);
            if (cVal)
                val = cVal;
            f.set(key, val);
        }
        result.push_back(f);
    }
    sqlite3_finalize(stmt);
}

pModel::FunctionList pModel::queryFunctions(oid ns_id, oid c_id, pStringRef name) const {

    FunctionList result;
    std::stringstream query;

    if (ns_id == pModel::NULLID)
        ns_id = getRootNamespaceOID();

    query << "SELECT name, type, flags, visibility, minArity, maxArity, " \
             " start_line, start_col, sourceModule.realPath FROM " \
             " function, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " " \
             " (namespace_id=" << ns_id << " OR namespace_id=" << getRootNamespaceOID() << ") AND class_id ";
    if (c_id) {
        query << " = " << c_id;
    }
    else {
        query << " IS NULL ";
    }

    query << " AND name='" << name.str() << "'";

    if (trace_) {
        std::cerr << "TRACE: " << query.str() << std::endl;
    }

    list_query<FunctionList>(query.str(), result);

    return result;

}

pModel::ClassList pModel::queryClasses(oid ns_id, pStringRef name) const {

    ClassList result;
    std::stringstream query;

    query << "SELECT class.id, name, type, flags, " \
             " start_line, start_col, sourceModule.realPath FROM " \
             " class, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " (namespace_id=" << ns_id << " OR namespace_id=1)" \
             " AND name='" << name.str() << "'";

    if (trace_) {
        std::cerr << "TRACE: " << query.str() << std::endl;
    }

    list_query<ClassList>(query.str(), result);

    return result;

}

pModel::ClassDeclList pModel::queryClassDecls(oid c_id, pStringRef name) const {

    ClassDeclList result;
    std::stringstream query;

    query << "SELECT class_decl.name, class.name AS className, class_decl.type, class_decl.flags, visibility, defaultVal, " \
             " class_decl.start_line, class_decl.start_col, sourceModule.realPath FROM " \
             " class_decl, class, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " class.id=class_decl.class_id AND" \
             " class_decl.class_id=" << c_id <<
             " AND class_decl.name='" << name.str() << "'";

    if (trace_) {
        std::cerr << "TRACE: " << query.str() << std::endl;
    }

    list_query<ClassDeclList>(query.str(), result);

    return result;

}

pModel::ConstantList pModel::queryConstants(pStringRef name) const {

    pModel::ConstantList result;
    std::stringstream query;

    query << "SELECT name, " \
             " start_line, start_col, sourceModule.realPath FROM " \
             " constant, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " type=" << pModel::DEFINE << " AND name='" << name.str() << "'";

    if (trace_) {
        std::cerr << "TRACE: " << query.str() << std::endl;
    }

    list_query<pModel::ConstantList>(query.str(), result);

    return result;

}

} // namespace

