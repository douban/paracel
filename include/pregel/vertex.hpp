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

#ifndef FILE_ace68a02_0c2c_1d7c_0dd7_3f42027dec00_HPP
#define FILE_ace68a02_0c2c_1d7c_0dd7_3f42027dec00_HPP

#include <string>
#include <unordered_map>
#include <functional>

#include "ps.hpp"
#include "graph.hpp"

namespace paracel {
namespace pregel {

template <class VertexVal, class MsgVal> // EdgeVal(type of edge) can only be 'double' in paracel
class vertex {
 
 public:
  vertex(Comm comm,
         std::string hosts_dct_str,
         std::string input,
         std::string output,
         paracel::parser_type parser,
         std::function<VertexVal(const std::string &)> init_func) {
    pt = new paralg(hosts_dct_str, comm, output);
    pt->paracel_load_as_graph(grp, input, parser, "fmap");
    init_vars(init_func);
  }
  
  vertex(Comm comm,
         std::string hosts_dct_str,
         std::string input,
         std::string output,
         paracel::parser_type parser,
         std::function<VertexVal(const std::string &,
                                 const std::string &,
                                 double)> init_func) {
    pt = new paralg(hosts_dct_str, comm, output);
    pt->paracel_load_as_graph(grp, input, parser, "fmap");
    init_vars(init_func);
  }

  virtual ~vertex() {
    delete pt;
  }

  void init_vars(std::function<VertexVal(const std::string &)> func) {
    auto func_lambda = [&] (const std::string & id) {
      vertex_val_map[id] = func(id);
    };
    grp.traverse_by_vertex(func_lambda);
    for(auto & kv : vertex_val_map) {
      std::string v = kv.first;
      vertex_adj_edge_val_map[v] = grp.adjacent(v);
      vertex_active_map[v] = true;
    }
  }
  
  void init_vars(std::function<VertexVal(const std::string &,
                                         const std::string &,
                                         double)> func) {
    auto func_lambda = [&] (const std::string & rid,
                            const std::string & cid,
                            double w) {
      vertex_val_map[rid] = func(rid, cid, w);
    };
    grp.traverse(func_lambda);
    for(auto & kv : vertex_val_map) {
      std::string v = kv.first;
      vertex_adj_edge_val_map[v] = grp.adjacent(v);
      vertex_active_map[v] = true;
    }
  }

  virtual void compute(const std::string & neighbor,
                       double wgt,
                       const VertexVal & val) = 0;

  virtual VertexVal mutable_value(const MsgVal & val) = 0;
  
  void vote_to_halt() {
    for(auto & kv : vertex_active_map) {
      std::string vtx = kv.first;
      VertexVal old_val = vertex_val_map[vtx];
      VertexVal new_val = mutable_value(pt->paracel_read<MsgVal>(vtx));
      vertex_val_map[vtx] = new_val; // local_update
      if(old_val == new_val) {
        vertex_active_map[vtx] = false;
      } else {
        vertex_active_map[vtx] = true;
      }
    }
  }

  void run_supersteps() {
    while(1) {
      int halt_flag = 1;
      for(auto & kv : vertex_active_map) {
        std::string vtx = kv.first;
        if(kv.second) {
          halt_flag = 0;
          for(auto & edge_info : vertex_adj_edge_val_map[vtx]) {
            // iter neighbors
            compute(edge_info.first, 
                    edge_info.second, 
                    vertex_val_map[vtx]);
          } // for
        } // if
      }
      sync();
      vote_to_halt();
      pt->get_comm().allreduce(halt_flag);
      if(halt_flag == pt->get_worker_size()) {
        break;
      }
      sync();
    }
    sync();
  }

  void dump_result(const std::string & prefix) {
    pt->paracel_dump_dict(vertex_val_map, prefix);
  }

 protected:
  paralg *pt;
  paracel::digraph<std::string> grp;
  std::unordered_map<std::string, VertexVal> vertex_val_map;
  std::unordered_map<std::string, std::unordered_map<std::string, double> > vertex_adj_edge_val_map;
  std::unordered_map<std::string, bool> vertex_active_map;
};

} // namespace pregel
} // namespace paracel

#endif
