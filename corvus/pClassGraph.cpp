/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "pClassGraph.h"
#include "pDB.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>

namespace corvus {

void pClassGraph::build_graph() {

    assert(graph_ == 0 && "already built graph");

    int num_vertices = (int)db_->sql_select_single_id("SELECT COUNT(DISTINCT lhs_class_id) FROM class_relations");

     graph_ = new GraphType(num_vertices);

     db::pDB::RowList result;
     const char *query = "SELECT lhs_class_id, rhs_class_id FROM class_relations";
     db_->list_query(query, result);
     for (int i = 0; i < result.size(); i++) {
         // to get direction from parent->child, rhs here on the left :/
         boost::add_edge(result[i].getAsInt("rhs_class_id"),
                         result[i].getAsInt("lhs_class_id"),
                     *graph_
                     );
     }

}

void pClassGraph::dump() {

    if (graph_)
        boost::print_graph(*graph_);

}

void pClassGraph::writeDot(pStringRef fileName) {

    if (!graph_)
        return;

    std::ofstream writer;
    writer.open(fileName.str().c_str());
    boost::write_graphviz(writer, *graph_);
    writer.close();

}


void pClassGraph::build() {

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
    const char *query = "SELECT C.id FROM class C LEFT JOIN class_model_decl CMD ON" \
             " C.id=CMD.class_id LEFT JOIN class_model_function CMF ON C.id=CMF.class_id" \
             " WHERE CMD.id IS NULL AND CMF.id IS NULL";
    db_->list_query(query, result);
    if (result.size() == 0)
        return;

    // even though classes are rebuilt individually, we still make a full
    // graph of the class heirarchy so we can determine the correct parents
    // for each class we rebuild
    build_graph();

}


}