/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/pModel.h"

#include <iostream>
#include <sstream>

namespace corvus { 

void pModel::sql_execute(pStringRef query) {
    char *errMsg;
    int rc = sqlite3_exec(db_, query.begin(), NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << query.str()
                  << "\n" << errMsg << "\n";
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

pModel::oid pModel::getSourceModule(pStringRef realPath) {

    if (modules_.find(realPath) != modules_.end()) {
        return modules_[realPath];
    }

    // XXX SELECT, CACHE

    std::stringstream ins;
    ins << "INSERT INTO sourceModule VALUES (NULL, '" << realPath.str() << "', '')";
    oid result = sql_insert(ins.str().c_str());
    modules_[realPath] = result;
    return result;

}

pModel::oid pModel::getNamespace(pStringRef ns) {

    if (namespaces_.find(ns) != namespaces_.end()) {
        return namespaces_[ns];
    }

    // XXX SELECT, CACHE

    std::stringstream ins;
    ins << "INSERT INTO namespace VALUES (NULL, '" << ns.str() << "')";
    oid result = sql_insert(ins.str().c_str());
    namespaces_[ns] = result;
    return result;

}

pModel::oid pModel::defineClass(pModel::oid ns, pStringRef name) {


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
                    int type, int flags, int vis, int sl, int sc,
                    int el, int ec) {

    std::stringstream ins;
    ins << "INSERT INTO function VALUES (NULL,"
        << m_id << ','
        << ns_id << ','
        << oidOrNull(c_id).str()
        << ",'" << name.str() << "',"
        << type << ','
        << flags << ','
        << vis << ','
        << sl  << ',' << sc  << ',' << el  << ',' << ec
        << ")";
    return sql_insert(ins.str().c_str());

}

void pModel::defineFunctionVar(oid f_id, pStringRef name,
                    int type, int flags, int datatype, pStringRef datatype_obj,
                    pStringRef defaultVal,
                    int sl, int sc) {

    std::stringstream ins;

    ins << "INSERT INTO function_var VALUES (NULL,"
        << f_id << ','
        << "'" << name.str() << "'" << ','
        << type << ','
        << flags << ','
        << datatype << ','
        << strOrNull(datatype_obj).str() << ","
        << strOrNull(defaultVal).str() << ","
        << sl  << ',' << sc
        << ")";
    sql_insert(ins.str().c_str());

}

} // namespace

