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

void pModel::sql_execute(pStringRef ref, pStringRef query) {
    char *errMsg;
    if (trace_) {
        std::cerr << "TRACE: " << query.str() << std::endl;
    }
    int rc = sqlite3_exec(db_, query.begin(), NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error in " << ref.str() << ": " << query.str()
                  << "\n" << errMsg << "\n";
        exit(1);
    }
}

pModel::oid pModel::sql_insert(pStringRef ref, pStringRef query) {
    sql_execute(ref, query);
    oid r = sqlite3_last_insert_rowid(db_);
    if (trace_) {
        std::cerr << "TRACE: insert id #" << r << std::endl;
    }
    return r;
}

void pModel::sql_setup() {
    sql_execute("sql_setup", "PRAGMA foreign_keys = ON");
    makeTables();
}

void pModel::makeTables() {

    const char *SM = "CREATE TABLE IF NOT EXISTS sourceModule (" \
                         "realpath TEXT UNIQUE," \
                         "hash TEXT" \
                         ")";
    sql_execute("makeTables:sourceModule", SM);

    const char *NS = "CREATE TABLE IF NOT EXISTS namespace (" \
                         "namespace TEXT UNIQUE" \
                         ")";
    sql_execute("makeTables:namespace", NS);
/*
    const char *CT = "CREATE TABLE IF NOT EXISTS context (" \
                         "module_id INTEGER," \
                         "parent_context_id INTEGER NULL," \
                         "scope INTEGER," \
                         "start_line INTEGER," \
                         "start_col INTEGER," \
                         "end_line INTEGER," \
                         "end_col INTEGER," \
                         "FOREIGN KEY(module_id) REFERENCES sourceModule(oid) ON DELETE CASCADE," \
                         "FOREIGN KEY(parent_context_id) REFERENCES context(oid) ON DELETE CASCADE"
                         ")";
    sql_execute("makeTables:context", CT);
*/
    const char *FU = "CREATE TABLE IF NOT EXISTS functions (" \
                         "context_id INTEGER NULL," \
                         "FOREIGN KEY(context_id) REFERENCES context(oid) ON DELETE CASCADE"
                         ")";
    sql_execute("makeTables:functions", FU);

}

pModel::oid pModel::getSourceModule(pStringRef realPath) {

    std::stringstream ins;
    ins << "INSERT INTO sourceModule VALUES ('" << realPath.str() << "', '')";
    return sql_insert("getSourceModule", ins.str().c_str());

}

pModel::oid pModel::getNamespace(pStringRef ns) {

    std::stringstream ins;
    ins << "INSERT INTO namespace VALUES ('" << ns.str() << "')";
    return sql_insert("getNamespace", ins.str().c_str());

}
/*
pModel::oid pModel::getContext(oid module_id, oid parent_context_id,
                               int scope, int start_line, int end_line,
                               int start_col, int end_col) {

    std::stringstream ins;
    ins << "INSERT INTO context VALUES (" << module_id << ",";
    if (parent_context_id)
        ins << parent_context_id;
    else
        ins << "NULL";
    ins << "," << scope << "," << start_line << "," << end_line
        << "," << start_col << "," << end_col << ")";
    return sql_insert("getContext", ins.str().c_str());

}
*/
} // namespace

