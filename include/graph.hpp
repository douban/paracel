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

#ifndef FILE_b92511cb_f391_2888_596e_99a4a6b74ee7_HPP
#define FILE_b92511cb_f391_2888_596e_99a4a6b74ee7_HPP

#include <tuple>
#include <utility>
#include <queue>
#include <algorithm>
#include <fstream>

#include "paracel_types.hpp"
#include "utils.hpp"

namespace paracel {

template <class T = size_t>
class undirected_graph {

 public:
  undirected_graph() {}
  
  undirected_graph(paracel::dict_type<T, paracel::dict_type<T, double> > edge_info) {
    construct_from_dict(edge_info);
  }

  undirected_graph(paracel::list_type<std::tuple<T, T> > tpls) {
    construct_from_tuples(tpls);
  }

  undirected_graph(paracel::list_type<std::tuple<T, T, double> > & tpls) {
    construct_from_triples(tpls);
  }

 private:
  void construct_from_dict(const paracel::dict_type<T, paracel::dict_type<T, double> > & edge_info) {
    for(auto & edge : edge_info) {
      for(auto & kv : edge.second) {
        add_edge(edge.first, kv.first, kv.second);
      }
    }
  } 

  void construct_from_tuples(const paracel::list_type<std::tuple<T, T> > & tpls) {
    for(auto & tpl : tpls) {
      add_edge(std::get<0>(tpl), std::get<1>(tpl));
    }
  }
      
  void construct_from_triples(paracel::list_type<std::tuple<T, T, double> > & tpls) {
    for(auto & tpl : tpls) {
      add_edge(std::get<0>(tpl), std::get<1>(tpl), std::get<2>(tpl));
    }
  }

 public:
  void add_edge(const T & v, const T & w) {
    add_edge(v, w, 1.);
  }

  void add_edge(const T & v, const T & w, double wgt) { 
    adj[v][w] = wgt;
    adj[w][v] = wgt;
    e_sz += 1;
    v_sz = adj.size();
  }

  paracel::dict_type<T, paracel::dict_type<T, double> > get_data() {
    return adj;
  }

  template <class F>
  void traverse(F & func) {
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        func(v.first, kv.first, kv.second);
      }
    }
  }

  template <class F>
  void traverse(const T & v, F & func) {
    if(!adj.count(v)) { return; }
    for(auto & kv : adj[v]) {
      func(v, kv.first, kv.second);
    }
  }

  template <class F>
  void traverse_by_vertex(F & func) {
    for(auto & v : adj) {
      func(v);
    }
  }

  inline size_t v() { 
    return v_sz; 
  }
  
  inline size_t e() { 
    return e_sz; 
  }
  
  paracel::dict_type<T, double> adjacent(const T & v) { 
    paracel::dict_type<T, double> empty_result;
    if(adj.count(v)) {
      return adj[v];
    }
    return empty_result;
  }
  
  inline size_t degree(const T & v) { 
    if(adj.count(v)) {
      return adj[v].size();
    }
    return 0;
  }
  
  inline double avg_degree() { 
    return 2. * e_sz / v_sz; 
  }
  
  inline size_t max_degree() {
    size_t max = 0;
    for(auto & v : adj) {
      if(degree(v.first) > max) {
        max = degree(v.first);
      }
    }
    return max;
  }
  
  std::vector<T> vertex_bag() {
    std::vector<T> r;
    for(auto & v : adj) {
      r.push_back(v.first);
    }
    return r;
  }

  std::unordered_set<T> vertex_set() {
    std::unordered_set<T> r;
    for(auto & v : adj) {
      r.insert(v.first);
    }
    return r;
  }
  
  inline int selfloops() {
    int cnt = 0;
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        if(v.first == kv.first) {
          cnt += 1;
        }
      }
    }
    return cnt / 2;
  }

 private:
  size_t v_sz = 0; 
  size_t e_sz = 0;
  //paracel::list_type<paracel::list_type<std::pair<size_t, double> > > adj;
  paracel::dict_type<T, paracel::dict_type<T, double> > adj;
 
 public:
  MSGPACK_DEFINE(v_sz, e_sz, adj);
};

template <class T = paracel::default_id_type>
class digraph {

public:
  digraph() {}

  digraph(paracel::dict_type<T, paracel::dict_type<T, double> > edge_info) {
    construct_from_dict(edge_info);
  }
  
  digraph(paracel::list_type<std::tuple<T, T> > tpls) {
    construct_from_tuples(tpls);
  }
  
  digraph(paracel::list_type<std::tuple<T, T, double> > tpls) {
    construct_from_triples(tpls);
  }

  void construct_from_dict(const paracel::dict_type<T, paracel::dict_type<T, double> > & edge_info) {
    for(auto & edge : edge_info) {
      for(auto & kv : edge.second)
      add_edge(edge.first, kv.first, kv.second);
    }
  }

  void construct_from_tuples(const paracel::list_type<std::tuple<T, T> > & tpls) {
    for(auto & tpl : tpls) {
      add_edge(std::get<0>(tpl), std::get<1>(tpl));
    }
  }
  
  void construct_from_triples(paracel::list_type<std::tuple<T, T, double> > & tpls) {
    for(auto & tpl : tpls) {
      add_edge(std::get<0>(tpl), std::get<1>(tpl), std::get<2>(tpl));
    }
  }
  
  void add_edge(const T & v, const T & w) {
    add_egde(v, w, 1.);
  }

  void add_edge(const T & v, const T & w, double wgt) {  
    adj[v][w] = wgt;
    if(!adj.count(w)) { adj[w].clear(); }
    reverse_adj[w][v] = wgt;
    e_sz += 1; // suppose no repeat
    v_sz = adj.size();
  }
  
  paracel::dict_type<T, paracel::dict_type<T, double> > get_data() {
    return adj;
  }

  template <class F>
  void traverse(F & func) {  
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        func(v.first, kv.first, kv.second);
      }
    }
  }

  template <class F>
  void traverse(const T & v, F & func) {
    for(auto & kv : adj[v]) {
      func(v, kv.first, kv.second);
    }
  }

  template <class F>
  void traverse_by_vertex(F & func) {
    for(auto & v : adj) {
      func(v);
    }
  }

  std::vector<T> vertex_bag() {
    std::vector<T> r;
    for(auto & v : adj) {
      r.push_back(v.first);
    }
    return r;
  }
  
  std::unordered_set<T> vertex_set() {
    std::unordered_set<T> r;
    for(auto & v : adj) {
      r.insert(v.first);
    }
    return r;
  }
  
  void dump2triples(paracel::list_type<std::tuple<T, T, double> > & tpls) {
    tpls.resize(0);
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        tpls.push_back(std::make_tuple(v.first, kv.first, kv.second));
      }
    }
  }

  void dump2dict(paracel::dict_type<T, paracel::dict_type<T, double> > & dict) {
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        dict[v.first][kv.first] = kv.second;
      }
    }
  }
 
  void reverse() {
    std::swap(adj, reverse_adj);
  }

  inline size_t v() { 
    return v_sz; 
  }
  
  inline size_t e() { 
    return e_sz; 
  }
  
  paracel::dict_type<T, double> 
  adjacent(const T & v) {
    return adj[v];
  }
  
  paracel::dict_type<T, double>
  reverse_adjacent(const T & v) {
    return reverse_adj[v];
  }

  inline size_t outdegree(const T & v) { 
    return adj[v].size(); 
  }

  inline size_t indegree(const T & vertex) {
    int cnt = 0;
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        if(vertex == kv.first) {
          cnt += 1;
        }
      }
    }
    return cnt;
  }
  
  inline double avg_degree() { 
    return static_cast<double>(e_sz / v_sz);
  }
  
  inline int selfloops() {
    int cnt = 0;
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        if(v.first == kv.first) {
          cnt += 1;
        }
      }
    }
    return cnt;
  }

 private:
  size_t v_sz = 0; 
  size_t e_sz = 0;
  paracel::dict_type<T, paracel::dict_type<T, double> > adj;
  paracel::dict_type<T, paracel::dict_type<T, double> > reverse_adj;
 
 public:
  MSGPACK_DEFINE(v_sz, e_sz, adj, reverse_adj);
}; // class digraph

template <class T = paracel::default_id_type>
class bigraph {

 public:
  bigraph() {}
  
  bigraph(paracel::dict_type<T, paracel::dict_type<T, double> > edge_info) {
    construct_from_dict(edge_info);
  }
  
  bigraph(paracel::list_type<std::tuple<T, T> > tpls) {
    construct_from_tuples(tpls);
  }
  
  bigraph(paracel::list_type<std::tuple<T, T, double> > tpls) {
    construct_from_triples(tpls);
  }
  
  void construct_from_dict(const paracel::dict_type<T, paracel::dict_type<T, double> > & edge_info) {
    for(auto & edge : edge_info) {
      for(auto & kv : edge.second)
      add_edge(edge.first, kv.first, kv.second);
    }
  }
  
  void construct_from_tuples(const paracel::list_type<std::tuple<T, T> > & tpls) {
    for(auto & tpl : tpls) {
      add_edge(std::get<0>(tpl), std::get<1>(tpl));
    }
  }
  
  void construct_from_triples(paracel::list_type<std::tuple<T, T, double> > & tpls) {
    for(auto & tpl : tpls) {
      add_edge(std::get<0>(tpl), std::get<1>(tpl), std::get<2>(tpl));
    }
  }
  
  void add_edge(const T & v, const T & w) {
    add_egde(v, w, 1.);
  }
  
  void add_edge(const T & v, const T & w, double wgt) {  
    adj[v][w] = wgt;
    v_sz = adj.size();
    e_sz += 1; // suppose no repeat
  }
  
  paracel::dict_type<T, paracel::dict_type<T, double> > get_data() { return adj; }

  template <class F>
  void traverse(F & func) {  
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        func(v.first, kv.first, kv.second);
      }
    }
  }
  
  template <class F>
  void traverse(const T & v, F & func) {
    for(auto & kv : adj[v]) {
      func(v, kv.first, kv.second);
    }
  }
  
  template <class F>
  void traverse_by_left_vertex(F & func) {
    for(auto & v : adj) {
      func(v);
    }
  }

  std::vector<T> left_vertex_bag() {
    std::vector<T> r;
    for(auto & v : adj) {
      r.push_back(v.first);
    }
    return r;
  }

  std::unordered_set<T> left_vertex_set() {
    std::unordered_set<T> r;
    for(auto & v : adj) {
      r.insert(v.first);
    }
    return r;
  }

  void dump2triples(paracel::list_type<std::tuple<T, T, double> > & tpls) {
    tpls.resize(0);
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        tpls.push_back(std::make_tuple(v.first, kv.first, kv.second));
      }
    }
  }

  void dump2dict(paracel::dict_type<T, paracel::dict_type<T, double> > & dict) {
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        dict[v.first][kv.first] = kv.second;
      }
    }
  }
 
  // left vertex size
  inline size_t v() { return v_sz; }
  
  inline size_t e() { return e_sz; }
  
  paracel::dict_type<T, double> adjacent(const T & v) { return adj[v]; }
  
  inline size_t outdegree(const T & v) { return adj[v].size(); }

  inline size_t indegree(const T & vertex) {
    int cnt = 0;
    for(auto & v : adj) {
      for(auto & kv : v.second) {
        if(vertex == kv.first) {
          cnt += 1;
        }
      }
    }
    return cnt;
  }
  
 private:
  size_t v_sz = 0; 
  size_t e_sz = 0;
  paracel::dict_type<T, paracel::dict_type<T, double> > adj;
 
 public:
  MSGPACK_DEFINE(v_sz, e_sz, adj);
}; // class bigraph

// continuous index from 0 to N-1
class bigraph_continuous {
 
 public:
  bigraph_continuous() {}

  bigraph_continuous(paracel::default_id_type n) {
    v_sz = n;
    adj.resize(v_sz);
    e_sz = 0;
  }

  // sequential interface only
  bigraph_continuous(const std::string & filename) {
    v_sz = 0;
    e_sz = 0;
    std::ifstream f(filename);
    if(!f) { throw std::runtime_error("open file error.\n"); }
    std::string line;
    std::getline(f, line);
    v_sz = std::stoi(line);
    adj.resize(v_sz);
    while(std::getline(f, line)) {
      auto lst = paracel::str_split(line, ',');
      add_edge(std::stoi(lst[0]), std::stoi(lst[1]), std::stod(lst[2]));
    }
    f.close();
  }

  void add_edge(paracel::default_id_type src, paracel::default_id_type dst) {
    add_edge(src, dst, 1.);
  }

  void add_edge(paracel::default_id_type src, paracel::default_id_type dst, double rating) {
    if(src >= (paracel::default_id_type)adj.size()) { adj.resize(src + 1); }
    adj[src].put(std::make_pair(dst, rating));
    e_sz += 1;
  }

  void resize(paracel::default_id_type n) {
    v_sz = n;
    adj.resize(v_sz);
    e_sz = 0;
  }

  paracel::default_id_type v() { 
    v_sz = adj.size();
    return v_sz; 
  }

  paracel::default_id_type e() { 
    return e_sz; 
  }

  paracel::bag_type<std::pair<paracel::default_id_type, double> >
  adjacent(paracel::default_id_type v) {
    return adj[v];
  }

  inline size_t outdegree(paracel::default_id_type v) {
    return adj[v].size();
  }

  inline size_t indegree(paracel::default_id_type v) {
    size_t cnt = 0;
    for(auto & v_bag : adj) {
      for(auto & item : v_bag) {
        if(item.first == v) {
          cnt += 1;
        }
      }
    }
    return cnt;
  }

  template <class F>
  void traverse(F & func) {
    for(paracel::default_id_type i = 0; i < v(); ++i) {
      for(auto & item : adj[i]) {
        func(i, item.first, item.second);
      }
    }
  }

  template <class F>
  void traverse(paracel::default_id_type v, F & func) {
    for(auto & item : adj[v]) {
      func(v, item.first, item.second);
    }
  }

 private:
  paracel::default_id_type v_sz = 0;
  paracel::default_id_type e_sz = 0;
  paracel::list_type<paracel::bag_type<std::pair<paracel::default_id_type, double> >  > adj;
 
 public:
  MSGPACK_DEFINE(v_sz, e_sz, adj);
}; // class bigraph_continuous

/*
template <class T = std::string>
using bigraph = paracel::digraph<T>;
*/

template <class G, class T, class F>
class DFS {

 public:
  DFS(G graph, T src, F lambda) {
    dfs(graph, src, lambda);
  }
  
  inline bool visited(const T & v) { return marked.count(v); }
 
 private:
  void dfs(G & graph, const T & v, F & func) {
    marked[v] = true;
    func(v);
    for(auto & kv : graph.adjacent(v)) {
      auto w = kv.first;
      if(!marked.count(w)) {
        dfs(graph, w, func);
      }
    }
  } // dfs
 
 private:
  paracel::dict_type<T, bool> marked;
}; // class DFS

template <class G, class T, class F>
class BFS {

 public:
  BFS(G graph, T src, F func) {
    bfs(graph, src, func);
  }
  
  inline bool visited(const T & v) { return marked.count(v); }
  
  inline T edgeTo(const T & v) { return edge_to[v]; }
  
  inline int dist(const T & v) { return dist_to[v]; }
 
 private:
  void bfs(G & grp, const T & v, F & func) {
    std::queue<T> q;
    q.push(v);
    marked[v] = true;
    func(v);
    dist_to[v] = 0;
    while(!q.empty()) {
      T node = q.front();
      q.pop();
      bool has_unmarked = false;
      for(auto & kv : grp.adjacent(node)) {
        auto w = kv.first;
        if(!marked.count(w)) {
          has_unmarked = true;
          q.push(w);
          marked[w] = true;
          func(w);
          edge_to[w] = node;
          dist_to[w] = dist_to[node] + 1;
        }
      }
    }
  }
 
 private:
  paracel::dict_type<T, bool> marked;
  paracel::dict_type<T, T> edge_to;
  paracel::dict_type<T, int> dist_to;
}; // class BFS

template <class G, class T>
class connected_components {
 
 public:
  connected_components(G graph) {
    paracel::list_type<T> vertexes = graph.vertex_bag();
    auto handler_lambda = [&] (const T & v) {
      identifier[v] = count;
      marked[v] = true;
    };
    for(auto & v : vertexes) {
      if(!marked.count(v)) {
        paracel::DFS<G, T, decltype(handler_lambda)> dfs_solver(graph, v, handler_lambda);
        count += 1;
      }
    }
  }

  bool is_connected(const T & v, const T & w) {
    return identifier(v) == identifier(w);
  }

  inline int cnt() { return count; }

  int id(const T & v) {
    if(!identifier.count(v)) {
      return -1;
    }
    return identifier[v];
  }

  template <class F>
  void handleCC(F & func) {
    for(auto & kv : identifier) {
      func(kv.first, kv.second);
    }
  }

 private:
  int count = 0;
  paracel::dict_type<T, int> identifier;
  paracel::dict_type<T, bool> marked;
}; // class Connected_Components

} // namespace paracel

#endif
