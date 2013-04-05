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

namespace corvus {

class pModel {
public:

    typedef sqlite3_int64 oid;

private:

    sqlite3 *db_;
    bool trace_;

    void sql_execute(pStringRef ref, pStringRef query);
    oid sql_insert(pStringRef ref, pStringRef query);
    void sql_setup();

    void makeTables();

public:

    pModel(sqlite3 *db, bool trace=false): db_(db), trace_(trace) {
        sql_setup();
    }

    oid getSourceModule(pStringRef realPath);
    oid getNamespace(pStringRef ns);
    /*
    oid getContext(oid module_id, oid parent_context_id,
                   int scope, int start_line, int end_line,
                   int start_col, int end_col);
                   */

};

} // namespace

#endif /* COR_PSOURCEFILE_H_ */
