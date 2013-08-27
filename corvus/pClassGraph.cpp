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
     std::pair<GraphType::edge_descriptor, bool> er;
     for (int i = 0; i < result.size(); i++) {
         // to get direction from parent->child, rhs here on the left :/
         db::pDB::oid parent = result[i].getAsOID("rhs_class_id");
         db::pDB::oid child = result[i].getAsOID("lhs_class_id");
         er = boost::add_edge(parent, child, *graph_);
         // cache vertexes if we don't have them yet
         if (vcache_.find(parent) == vcache_.end()) {
             vcache_[parent] = boost::source(er.first, *graph_);
         }
         if (vcache_.find(child) == vcache_.end()) {
             vcache_[child] = boost::target(er.first, *graph_);
         }
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

void pClassGraph::cache_class_decls(db::pDB::oid in_class_id, db::pDB::oid parent_class_id) {

    //    work_id = (parent_class_id != NULLID) ? parent_class_id : in_class_id
    db::pDB::oid work_id = (parent_class_id != db::pDB::NULLID) ? parent_class_id : in_class_id;
    if (db_->trace())
        std::cout << "cache_class_decl(" << work_id << "," << parent_class_id << ")\n";

    // find vertex in the graph
    assert(vcache_.find(work_id) != vcache_.end() && "vertex not in cache");

    //    [LOOP] for each method|decl X in work_id list that doesn't match a symbol already in cache for in_class_id
    //      cache X for in_class_id
    db::pDB::RowList result;
    std::stringstream query, iquery;
    query << "SELECT id FROM function WHERE class_id=" << work_id;
    db_->list_query(query.str(), result);
    for (int i = 0; i < result.size(); ++i) {
        iquery << "INSERT INTO class_model_function VALUES (NULL, "
               << in_class_id << ","
               << result[i].getAsOID("id") << ")";
        db_->sql_execute(iquery.str());
        iquery.str("");
    }


    query.str(""); result.clear();
    query << "SELECT id FROM class_decl WHERE class_id=" << work_id;
    db_->list_query(query.str(), result);
    for (int i = 0; i < result.size(); ++i) {
        iquery << "INSERT INTO class_model_decl VALUES (NULL, "
               << in_class_id << ","
               << result[i].getAsOID("id") << ")";
        db_->sql_execute(iquery.str());
        iquery.str("");
    }

    //    [LOOP] for each parent P of work_id
    //      recurse [in_class_id P]
    GraphType::vertex_descriptor source = vcache_[work_id];
    typedef GraphType::in_edge_iterator parent_iter;
    parent_iter iter, end;
    for (boost::tie(iter,end) = boost::in_edges(source, *graph_);
         iter != end;
         ++iter) {
        std::cout << "parent: " << boost::source(*iter, *graph_) << std::endl;
        cache_class_decls(in_class_id, boost::source(*iter, *graph_));
    }

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
    const char *query = "SELECT C.id AS in_class_id FROM class C LEFT JOIN class_model_decl CMD ON" \
             " C.id=CMD.class_id LEFT JOIN class_model_function CMF ON C.id=CMF.class_id" \
             " WHERE CMD.id IS NULL AND CMF.id IS NULL";
    db_->list_query(query, result);
    if (result.size() == 0)
        return;

    // so we'll build the full graph based on all edges in class_relations
    build_graph();

    db_->begin();

    // cache decls for each class we're interested in
    for (int i = 0; i < result.size(); i++) {
        cache_class_decls(result[i].getAsOID("in_class_id"), db::pDB::NULLID);
    }

    db_->commit();

}


}