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

#ifndef FILE_2eeea3bc_a8b7_1710_aff7_8a0501fd89b1_HPP
#define FILE_2eeea3bc_a8b7_1710_aff7_8a0501fd89b1_HPP

#include <string>
#include <iostream>
#include <unordered_map>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"
#include "graph.hpp"

namespace paracel {
namespace alg {

//using node_t = paracel::default_id_type;

class max_vertex_value : public paracel::paralg {
 
 public:
  max_vertex_value(paracel::Comm comm,
                   std::string hosts_dct_str,
                   std::string _input,
                   std::string _output,
                   std::string update_fn,
                   std::string update_fcn) : 
      paracel::paralg(hosts_dct_str, comm, _output), 
      input(_input),
      output(_output),
      update_file(update_fn),
      update_funcs(update_fcn) {}

  virtual ~max_vertex_value() {}
  
  void init() {
    auto local_parser = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    auto f_parser = paracel::gen_parser(local_parser);
    paracel_load_as_graph(grp, input, f_parser, "fmap");
    // init vertex_val_map
    auto lambda = [&] (const std::string & rid,
                       const std::string & cid,
                       double wgt) {
      vertex_val_map[rid] = std::stod(rid);
    };
    grp.traverse(lambda);
    // init vertex_adj_edge_val_map & vertex_active_map
    for(auto & vertex : vertex_val_map) {
      vertex_adj_edge_val_map[vertex.first] = grp.adjacent(vertex.first);
      vertex_active_map[vertex.first] = true;
    }
  }
  
  void solve() {
    init();
    // superstep 0
    for(auto & kv : vertex_val_map) {
      paracel_bupdate(kv.first, 
                      kv.second, 
                      update_file,
                      update_funcs);
    }
    sync();
    // following supersteps
    while(1) {
      size_t local_halt_flag = 1;
      // following supersteps
      for(auto & kv : vertex_active_map) {
        std::string v = kv.first;
        // if vertex is active
        if(kv.second) {
          local_halt_flag = 0;
          // iter outgoing edges
          for(auto & edge_info : vertex_adj_edge_val_map[v]) {
            std::string link_v = edge_info.first;
            paracel_bupdate(link_v, 
                            vertex_val_map[v], 
                            update_file,
                            update_funcs);
          }
        }
      }
      sync();

      // update vertex_active_map
      for(auto & kv : vertex_active_map) {
        std::string vertex = kv.first;
        double new_val = paracel_read<double>(vertex);
        double old_val = vertex_val_map[vertex];
        vertex_val_map[vertex] = new_val; // local update
        if(new_val == old_val) {
          vertex_active_map[vertex] = false; // vote to halt
        } else {
          vertex_active_map[vertex] = true; // reactive
        }
      }
      sync();
      get_comm().allreduce(local_halt_flag);
      if(local_halt_flag == get_worker_size()) {
        break;
      }
      sync();
    }
    sync();
  } // solve

  void dump_result() {
    /*
    for(auto & kv : vertex_val_map) {
      std::cout << kv.first << ": " << kv.second << std::endl;
    }
    */
    paracel_dump_dict(vertex_val_map, "vertex_val_");
  }

 private:
  std::string input, output;
  std::string update_file, update_funcs;
  paracel::digraph<std::string> grp;
  std::unordered_map<std::string, double> vertex_val_map;
  std::unordered_map<std::string, std::unordered_map<std::string, double> > vertex_adj_edge_val_map;
  std::unordered_map<std::string, bool> vertex_active_map;
}; // class max_vertex_val

} // namespace alg
} // namespace paracel

#endif
