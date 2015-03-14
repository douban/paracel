/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Distributed under the BSD License. Check out the LICENSE file for full text.
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone https://github.com/douban/paracel.git 
 *
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE GRAPH_TEST 

#include <vector>
#include <string>
#include <iostream>
#include <tuple>
#include <boost/test/unit_test.hpp>
#include "graph.hpp"
#include "paracel_types.hpp"

BOOST_AUTO_TEST_CASE (bigraph_test) {
  
  paracel::list_type<std::tuple<std::string, std::string, double> > tpls;
  tpls.emplace_back(std::make_tuple("a", "A", 3.));
  tpls.emplace_back(std::make_tuple("a", "B", 4.));
  tpls.emplace_back(std::make_tuple("a", "D", 2.));
  tpls.emplace_back(std::make_tuple("b", "C", 5.));
  tpls.emplace_back(std::make_tuple("b", "D", 4.));
  tpls.emplace_back(std::make_tuple("c", "D", 3.));
  tpls.emplace_back(std::make_tuple("b", "E", 5.));
  tpls.emplace_back(std::make_tuple("c", "E", 1.));
  tpls.emplace_back(std::make_tuple("c", "F", 2.));
  tpls.emplace_back(std::make_tuple("d", "C", 3.));
  
  paracel::bigraph<std::string> grp(tpls);
  
  auto data = grp.get_data();
  
  auto print_lambda = [] (std::string u, std::string v, double wgt) {
    std::cout << u << "|" << v << "|" << wgt << std::endl;
  };
  grp.traverse(print_lambda);
  
  grp.add_edge("c", "G", 3.6);
  grp.traverse("c", print_lambda);

  auto ubag = grp.left_vertex_bag();
  auto uset = grp.left_vertex_set();

  paracel::list_type<std::tuple<std::string, std::string, double> > dump_tpls;
  grp.dump2triples(dump_tpls);

  paracel::dict_type<std::string, paracel::dict_type<std::string, double> > dump_dict;
  grp.dump2dict(dump_dict);

  BOOST_CHECK_EQUAL(grp.v(), 4);
  BOOST_CHECK_EQUAL(grp.e(), 11);
  BOOST_CHECK_EQUAL(grp.outdegree("a"), 3);
  BOOST_CHECK_EQUAL(grp.indegree("E"), 2);

  auto adj_info = grp.adjacent("c");
}

BOOST_AUTO_TEST_CASE (bigraph_continuous_test) {
  
  paracel::bigraph_continuous G;
  G.add_edge(0, 1, 3.);
  G.add_edge(0, 2, 4.);
  G.add_edge(0, 4, 2.);
  G.add_edge(1, 3, 5.);
  G.add_edge(1, 4, 4.);
  G.add_edge(1, 5, 5.);
  G.add_edge(2, 4, 3.);
  G.add_edge(2, 5, 1.);
  G.add_edge(2, 6, 2.);
  G.add_edge(3, 3, 3.);
  
  auto print_lambda = [] (paracel::default_id_type u, paracel::default_id_type v, double wgt) {
    std::cout << u << "|" << v << "|" << wgt << std::endl;
  };
  G.traverse(print_lambda);
  G.traverse(0, print_lambda);

  BOOST_CHECK_EQUAL(G.v(), 4);
  BOOST_CHECK_EQUAL(G.e(), 10);
  BOOST_CHECK_EQUAL(G.outdegree(0), 3);
  BOOST_CHECK_EQUAL(G.indegree(5), 2);

  auto adj_info = G.adjacent(2);

}

BOOST_AUTO_TEST_CASE (digraph_test) {
  paracel::list_type<std::tuple<size_t, size_t, double> > tpls;
  tpls.emplace_back(std::make_tuple(0, 0, 3.));
  tpls.emplace_back(std::make_tuple(0, 2, 5.));
  tpls.emplace_back(std::make_tuple(1, 0, 4.));
  tpls.emplace_back(std::make_tuple(1, 1, 3.));
  tpls.emplace_back(std::make_tuple(1, 2, 1.));
  tpls.emplace_back(std::make_tuple(2, 0, 2.));
  tpls.emplace_back(std::make_tuple(2, 3, 1.));
  tpls.emplace_back(std::make_tuple(3, 1, 3.));
  tpls.emplace_back(std::make_tuple(3, 3, 1.));
  paracel::digraph<size_t> grp(tpls);
  paracel::digraph<size_t> grp2;
  BOOST_CHECK_EQUAL(grp2.v(), 0);
  grp2.construct_from_triples(tpls);
  BOOST_CHECK_EQUAL(grp2.v(), 4);

  grp.add_edge(3, 4, 5.);

  auto data = grp.get_data();
  auto vbag = grp.vertex_bag();
  auto vset = grp.vertex_set();
  auto adj = grp.adjacent(0);

  auto f = [] (size_t a, size_t b, double c) {
    std::cout << a << " | " << b << " | " << c << std::endl;
  };
  grp.traverse(f);
  grp.traverse(1, f);
  
  grp2.traverse(f);
  grp2.traverse(1, f);
  
  paracel::list_type<std::tuple<size_t, size_t, double> > dump_tpls;
  grp.dump2triples(dump_tpls);
  paracel::dict_type<size_t, paracel::dict_type<size_t, double> > dump_dict;
  grp.dump2dict(dump_dict);

  BOOST_CHECK_EQUAL(grp.v(), 5);
  BOOST_CHECK_EQUAL(grp.e(), 10);
  BOOST_CHECK_EQUAL(grp.outdegree(0), 2);
  BOOST_CHECK_EQUAL(grp.indegree(0), 3);
  BOOST_CHECK_EQUAL(grp.avg_degree(), 2.);
  BOOST_CHECK_EQUAL(grp.selfloops(), 3);

  grp.reverse();
  BOOST_CHECK_EQUAL(grp.v(), 5);
  BOOST_CHECK_EQUAL(grp.e(), 10);
  BOOST_CHECK_EQUAL(grp.outdegree(0), 3);
  BOOST_CHECK_EQUAL(grp.indegree(0), 2);
  BOOST_CHECK_EQUAL(grp.avg_degree(), 2.);
  BOOST_CHECK_EQUAL(grp.selfloops(), 3);
}

BOOST_AUTO_TEST_CASE (undirected_graph_test) {
    paracel::list_type<std::tuple<size_t, size_t> > edges;
    edges.emplace_back(std::make_tuple(0, 1));
    edges.emplace_back(std::make_tuple(0, 2));
    edges.emplace_back(std::make_tuple(0, 5));
    edges.emplace_back(std::make_tuple(0, 6));
    edges.emplace_back(std::make_tuple(3, 4));
    edges.emplace_back(std::make_tuple(3, 5));
    edges.emplace_back(std::make_tuple(4, 5));
    edges.emplace_back(std::make_tuple(4, 6));
    edges.emplace_back(std::make_tuple(9, 10));
    edges.emplace_back(std::make_tuple(9, 11));
    edges.emplace_back(std::make_tuple(9, 12));
    edges.emplace_back(std::make_tuple(11, 12));
    paracel::undirected_graph<size_t> grp(edges);
    BOOST_CHECK_EQUAL(grp.v(), 11);
    BOOST_CHECK_EQUAL(grp.e(), 12);
    auto adj = grp.adjacent(5); // {0: 1.0, 4: 1.0}
    BOOST_CHECK_EQUAL(grp.avg_degree(), 24. / 11.);
    BOOST_CHECK_EQUAL(grp.max_degree(), 4);
    BOOST_CHECK_EQUAL(grp.selfloops(), 0);
}
