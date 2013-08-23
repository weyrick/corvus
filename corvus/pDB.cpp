/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "pDB.h"

#include <iostream>
#include <sstream>

namespace corvus { 

namespace db {

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

void pDB::sql_execute(pStringRef query) const {
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

pDB::oid pDB::sql_insert(pStringRef query) const {
    sql_execute(query);
    oid r = sqlite3_last_insert_rowid(db_);
    if (trace_) {
        std::cerr << "TRACE: insert id #" << r << std::endl;
    }
    return r;
}

pDB::oid pDB::sql_select_single_id(pStringRef query) const {

    sqlite3_stmt *stmt;
    pDB::oid result = pDB::NULLID;

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
        if (trace_)
            std::cerr << "TRACE: found 1 row\n";
    }
    else if (trace_) {
        std::cerr << "TRACE: no rows found\n";
    }

    sqlite3_finalize(stmt);

    return result;

}


std::string pDB::sql_select_single_string(pStringRef query) const {

    sqlite3_stmt *stmt;
    std::string result;

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
        result = (char*)sqlite3_column_text(stmt, 0);
        if (trace_)
            std::cerr << "TRACE: found 1 row\n";
    }
    else if (trace_) {
        std::cerr << "TRACE: no rows found\n";
    }

    sqlite3_finalize(stmt);

    return result;

}


void pDB::sql_setup() {
    sql_execute("PRAGMA foreign_keys = ON");
    sql_execute("BEGIN");
}

void pDB::commit(bool begin) {
    sql_execute("COMMIT");
    if (begin)
        sql_execute("BEGIN");
}


std::string pDB::oidOrNull(oid val) {
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
std::string pDB::sql_string(pStringRef val, bool allowNull) {
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

//template <typename LTYPE>
void pDB::list_query(pStringRef query, RowList &result) const {
    sqlite3_stmt *stmt;
    if (trace_) {
        std::cerr << "TRACE: " << query.str() << "\n";
    }
    int rc = sqlite3_prepare_v2(db_, query.str().c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite error: " << query.str() << "\n";
        std::cerr << sqlite3_errmsg(db_) << "\n";
        exit(1);
    }
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        //typename LTYPE::value_type f;
        RowList::value_type f;
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
    if (trace_) {
        std::cerr << "TRACE: " << result.size() << " rows returned\n";
    }
    sqlite3_finalize(stmt);
}


} } // namespace

