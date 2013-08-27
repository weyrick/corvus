/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef PCLASSGRAPH_H
#define PCLASSGRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <map>

#include "pDB.h"
#include "corvus/pTypes.h"

namespace corvus {

class pClassGraph
{
public:
    typedef boost::adjacency_list<
       boost::mapS, boost::vecS, boost::bidirectionalS,
       boost::property<boost::vertex_color_t, boost::default_color_type,
           boost::property<boost::vertex_degree_t, db::pDB::oid,
             boost::property<boost::vertex_in_degree_t, db::pDB::oid,
       boost::property<boost::vertex_out_degree_t, db::pDB::oid> > > >
     > GraphType;

private:

    // we do not own
    db::pDB* db_;

    // own
    GraphType *graph_;

    // cache of vertexes by id
    std::map<db::pDB::oid, GraphType::vertex_descriptor> vcache_;

    void build_graph();

    void cache_class_decls(db::pDB::oid in_class_id, db::pDB::oid parent_class_id);

public:
    pClassGraph(db::pDB* db): db_(db), graph_(0) { }

    ~pClassGraph() { if (graph_) delete graph_; }

    void build();

    void dump();
    void writeDot(pStringRef fileName);


};

} // namespace

#endif // PCLASSGRAPH_H
