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

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/graph/visitors.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>

namespace corvus {

namespace {
template <class ParentDecorator>
struct print_parent {
  print_parent(const ParentDecorator& p_) : p(p_) { }
  template <class Vertex>
  void operator()(const Vertex& v) const {
    std::cout << "parent[" << v << "] = " <<  p[v]  << std::endl;
  }
  ParentDecorator p;
};


template <class NewGraph, class Tag>
struct graph_copier
  : public boost::base_visitor<graph_copier<NewGraph, Tag> >
{
  typedef Tag event_filter;

  graph_copier(NewGraph& graph) : new_g(graph) { }

  template <class Edge, class Graph>
  void operator()(Edge e, Graph& g) {
    boost::add_edge(boost::source(e, g), boost::target(e, g), new_g);
  }
private:
  NewGraph& new_g;
};

template <class NewGraph, class Tag>
inline graph_copier<NewGraph, Tag>
copy_graph(NewGraph& g, Tag) {
  return graph_copier<NewGraph, Tag>(g);
}
}

void pClassModelBuilder::build_graph() {


    typedef boost::adjacency_list<
       boost::mapS, boost::vecS, boost::directedS,
       boost::property<boost::vertex_color_t, boost::default_color_type,
           boost::property<boost::vertex_degree_t, int,
             boost::property<boost::vertex_in_degree_t, int,
       boost::property<boost::vertex_out_degree_t, int> > > >
     > Graph;
    //typedef boost::adjacency_list< boost::mapS, boost::vecS, boost::directedS > Graph;

    int num_vertices = (int)db_->sql_select_single_id("SELECT COUNT(DISTINCT lhs_class_id) FROM class_relations");

     Graph G(num_vertices);

     db::pDB::RowList result;
     const char *query = "SELECT lhs_class_id, rhs_class_id FROM class_relations";
     db_->list_query(query, result);
     for (int i = 0; i < result.size(); i++) {
         // to get direction from parent->child, rhs here on the left :/
         boost::add_edge(result[i].getAsInt("rhs_class_id"),
                         result[i].getAsInt("lhs_class_id"),
                     G
                     );
     }

     typedef Graph::vertex_descriptor Vertex;

     //Graph G_copy(num_vertices);
     // Array to store predecessor (parent) of each vertex. This will be
     // used as a Decorator (actually, its iterator will be).
     std::vector<Vertex> p(boost::num_vertices(G));
     // VC++ version of std::vector has no ::pointer, so
     // I use ::value_type* instead.
     typedef std::vector<Vertex>::value_type *Piter;

     // Array to store distances from the source to each vertex .  We use
     // a built-in array here just for variety. This will also be used as
     // a Decorator.
     //boost::graph_traits<Graph>::vertices_size_type d[num_vertices];
     //std::fill_n(d, num_vertices, 0);

     // The source vertex
     Vertex s = *(boost::vertices(G).first);
     p[s] = s;
     /*
     boost::breadth_first_search
       (G, s,
        boost::visitor(boost::make_bfs_visitor
        (std::make_pair
                        (boost::record_predecessors(&p[0],
                                                    boost::on_tree_edge()),
                         copy_graph(G_copy, boost::on_examine_edge())))) );*/

     boost::breadth_first_search
       (G, s,
        boost::visitor(boost::make_bfs_visitor
        (boost::record_predecessors(&p[0], boost::on_tree_edge()))));

     //boost::print_graph(G);
     //boost::write_graphviz(std::cout, G);
     std::ofstream writer;
     writer.open("out.dot");
     boost::write_graphviz(writer, G);
     writer.close();
     //boost::print_graph(G_copy);


     //std::for_each(boost::vertices(G).first, boost::vertices(G).second,
       //            print_parent<Piter>(&p[0]));

     /*
     if (boost::num_vertices(G) < 11) {
       std::cout << "distances: ";
       std::copy(d, d + 5, std::ostream_iterator<int>(std::cout, " "));
       std::cout << std::endl;

       std::for_each(boost::vertices(G).first, boost::vertices(G).second,
                     print_parent<Piter>(&p[0]));
     }
    */

}

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