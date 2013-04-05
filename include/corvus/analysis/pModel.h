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
    sqlite3_int64 namespace_id_;

    sqlite3 *db_;

    void sql_execute(pStringRef query);
    oid sql_insert(pStringRef query);

public:

    pModel(sqlite3 *db): db_(db) { }

    oid getSourceModule(pStringRef realPath);

};

} // namespace

#endif /* COR_PSOURCEFILE_H_ */
