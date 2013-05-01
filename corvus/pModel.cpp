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

namespace corvus { 

void pModel::sql_execute(pStringRef query) {
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

pModel::oid pModel::sql_insert(pStringRef query) {
    sql_execute(query);
    oid r = sqlite3_last_insert_rowid(db_);
    if (trace_) {
        std::cerr << "TRACE: insert id #" << r << std::endl;
    }
    return r;
}

pModel::oid pModel::sql_select_single_id(pStringRef query) {

    sqlite3_stmt *stmt;
    pModel::oid result = pModel::NULLID;

    int rc = sqlite3_prepare_v2(db_, query.str().c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << query.str() << "\n";
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
    const char *SD = "CREATE TABLE IF NOT EXISTS static (" \
                         "id INTEGER PRIMARY KEY,"
                         "sourceModule_id INTEGER NOT NULL," \
                         "type INTEGER NOT NULL," \
                         "value TEXT NOT NULL," \
                         "line INTEGER NOT NULL," \
                         "col INTEGER NOT NULL," \
                         "FOREIGN KEY(sourceModule_id) REFERENCES sourceModule(id) ON DELETE CASCADE"
                         ")";
    sql_execute(SD);

    const char *SU = "CREATE TABLE IF NOT EXISTS static_use (" \
                         "id INTEGER PRIMARY KEY,"
                         "static_id INTEGER NOT NULL," \
                         "line INTEGER NOT NULL," \
                         "col INTEGER NOT NULL," \
                             "FOREIGN KEY(static_id) REFERENCES static(id) ON DELETE CASCADE"
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
                         "line INTEGER NOT NULL," \
                         "col INTEGER NOT NULL," \
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
                         "line INTEGER NOT NULL," \
                         "col INTEGER NOT NULL," \
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
                         "line INTEGER NOT NULL," \
                         "col INTEGER NOT NULL," \
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
                         "line INTEGER NOT NULL," \
                         "col INTEGER NOT NULL," \
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
                         "line INTEGER NOT NULL," \
                         "col INTEGER NOT NULL," \
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

pModel::oid pModel::getNamespaceOID(pStringRef ns) {

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

    sql.str("");

    sql << "INSERT INTO namespace VALUES (NULL, '" << ns.str() << "')";
    oid result = sql_insert(sql.str().c_str());
    namespaces_[ns] = result;
    return result;

}

pModel::oid pModel::defineClass(pModel::oid ns_id, pModel::oid m_id, pStringRef name) {

/*
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
*/

    int type = pModel::CLASS;
    int flags = pModel::NO_FLAGS;
    int sl = 0;
    int sc = 0;
    int el = 0;
    int ec = 0;

    std::stringstream sql;
    sql << "INSERT INTO class VALUES (NULL,"
        << m_id << ','
        << ns_id
        << ",'" << name.str() << "',"
        << type << ','
        << flags << ','
        << sl  << ',' << sc  << ',' << el  << ',' << ec
        << ",'',''" // extends, implements
        << ")";
    return sql_insert(sql.str().c_str());

}

pStringRef pModel::oidOrNull(oid val) {
    if (val) {
        std::stringstream conv;
        conv << val;
        return conv.str();
    }
    else {
        return "NULL";
    }
}

pStringRef pModel::strOrNull(pStringRef val) {
    if (!val.empty()) {
        std::stringstream conv;
        conv << '\'' << val.str() << '\'';
        return conv.str();
    }
    else {
        return "NULL";
    }
}

pModel::oid pModel::defineFunction(oid ns_id, oid m_id, oid c_id, pStringRef name,
                    int type, int flags, int vis, int minA, int maxA, int sl, int sc,
                    int el, int ec) {

    std::stringstream sql;
    sql << "INSERT INTO function VALUES (NULL,"
        << m_id << ','
        << ns_id << ','
        << oidOrNull(c_id).str()
        << ",'" << name.str() << "',"
        << type << ','
        << flags << ','
        << vis << ','
        << minA << ','
        << maxA << ','
        << sl  << ',' << sc  << ',' << el  << ',' << ec
        << ")";
    return sql_insert(sql.str().c_str());

}

void pModel::defineFunctionVar(oid f_id, pStringRef name,
                    int type, int flags, int datatype, pStringRef datatype_obj,
                    pStringRef defaultVal,
                    int sl, int sc) {

    std::stringstream sql;

    sql << "INSERT INTO function_var VALUES (NULL,"
        << f_id << ','
        << "'" << name.str() << "'" << ','
        << type << ','
        << flags << ','
        << datatype << ','
        << strOrNull(datatype_obj).str() << ","
        << strOrNull(defaultVal).str() << ","
        << sl  << ',' << sc
        << ")";
    sql_insert(sql.str().c_str());

}

pModel::FunctionList pModel::queryFunctions(oid ns_id, oid c_id, pStringRef name) {

    FunctionList result;
    std::stringstream query;

    query << "SELECT name, type, flags, visibility, minArity, maxArity, " \
             " start_line, start_col, sourceModule.realPath FROM " \
             " function, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " " \
             " (namespace_id=" << ns_id << " OR namespace_id=1) AND class_id ";
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

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db_, query.str().c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << query.str() << "\n";
        exit(1);
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        model::function f;
        f.name = (char*)sqlite3_column_text(stmt, 0);
        f.ftype = sqlite3_column_int(stmt, 1);
        f.flags = sqlite3_column_int(stmt, 2);
        f.visibility = sqlite3_column_int(stmt, 3);
        f.minArity = sqlite3_column_int(stmt, 4);
        f.maxArity = sqlite3_column_int(stmt, 5);
        f.startLine = sqlite3_column_int(stmt, 6);
        f.startCol = sqlite3_column_int(stmt, 7);
        f.sourceModule = (char*)sqlite3_column_text(stmt, 8);
        result.push_back(f);
    }

    sqlite3_finalize(stmt);

    return result;

}

} // namespace

