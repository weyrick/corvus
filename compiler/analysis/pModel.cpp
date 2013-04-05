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
        std::cerr << "sqlite error: " << errMsg << "\n";
        exit(1);
    }
}

pModel::oid pModel::sql_insert(pStringRef query) {
    sql_execute(query);
    return sqlite3_last_insert_rowid(db_);
}

pModel::oid pModel::getSourceModule(pStringRef realPath) {

    const char *CREATE = "CREATE TABLE IF NOT EXISTS sourceModules (" \
                         "realpath TEXT UNIQUE," \
                         "hash TEXT" \
                         ")";
    sql_execute(CREATE);

    std::stringstream ins;
    ins << "INSERT INTO sourceModules VALUES ('" << realPath.str() << "', '')";
    oid i = sql_insert(ins.str().c_str());
    std::cout << "insert rowid " << i << std::endl;
    return i;

}

} // namespace

