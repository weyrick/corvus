/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "pClassModelBuilder.h"
#include "pDB.h"

#include <sstream>

namespace corvus {

void pClassModelBuilder::refresh() {

    // the class model is essentially a cache of all properties, consts and methods
    // that each class contains, considering the full class heirarchy as it
    // is currently known through resolved class relations

    // this assumes that resolveClassRelations has already been run

    // only classes that aren't already in the class model are rebuilt.
    // since class model data cascade deletes when its parent class is deleted,
    // (and classes in turn are deleted when their parent source modules are)
    // this ensures an accurate model

    // first, see if any classes need their models rebuilt. these will be the
    // list of classes that have (rows(class_model_decl)==0 && rows(class_model_function)==0)
    // if there are none to rebuild, we're done (and we can skip the class heirarchy graph build)
    db::pDB::RowList result;
    std::stringstream query;
    query << "SELECT C.id FROM class C LEFT JOIN class_model_decl CMD ON" \
             " C.id=CMD.class_id LEFT JOIN class_model_function CMF ON C.id=CMF.class_id" \
             " WHERE CMD.id IS NULL AND CMF.id IS NULL";
    //db_->list_query<RowList>(query.str(), result);
    db_->list_query(query.str(), result);
    if (result.size() == 0)
        return;

    // even though classes are rebuilt individually, we still make a full
    // graph of the class heirarchy so we can determine the correct parents
    // for each class we rebuild

    query.str("");

}


}