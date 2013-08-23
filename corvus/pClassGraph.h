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

#include "corvus/pTypes.h"

namespace corvus {

namespace db { class pDB; }

class pClassGraph
{
public:
    typedef boost::adjacency_list<
       boost::mapS, boost::vecS, boost::directedS,
       boost::property<boost::vertex_color_t, boost::default_color_type,
           boost::property<boost::vertex_degree_t, int,
             boost::property<boost::vertex_in_degree_t, int,
       boost::property<boost::vertex_out_degree_t, int> > > >
     > GraphType;

private:

    // we do not own
    db::pDB* db_;

    GraphType *graph_;

    void build_graph();

public:
    pClassGraph(db::pDB* db): db_(db), graph_(0) { }

    ~pClassGraph() { if (graph_) delete graph_; }

    void build();

    void dump();
    void writeDot(pStringRef fileName);


};

} // namespace

#endif // PCLASSGRAPH_H
