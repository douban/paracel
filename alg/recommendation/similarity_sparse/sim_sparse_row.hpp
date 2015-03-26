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

#ifndef FILE_9bb637f5_938a_bdc6_a5eb_393c7f112402_HPP
#define FILE_9bb637f5_938a_bdc6_a5eb_393c7f112402_HPP

#include <cmath>
#include <queue>
#include <vector>
#include <string>
#include <unordered_map>
#include "utils.hpp"
#include "ps.hpp"

namespace paracel {
namespace alg {

using node_t = paracel::default_id_type;

struct min_heap_cmp {
  inline bool operator() (const std::pair<node_t, double> & l,
                          const std::pair<node_t, double> & r) {
    return l.second > r.second;
  }
};

using min_heap = std::priority_queue<std::pair<node_t, double>,
                                    std::vector<std::pair<node_t, double> >,
                                    min_heap_cmp>;

class sim_sparse_row : public paracel::paralg {
 public:
  sim_sparse_row(paracel::Comm comm,
                 std::string hosts_dct_str,
                 std::string _input,
                 std::string _output,
                 std::string _fmt,
                 int _topk,
                 int _rbar,
                 int _cbar,
                 int _rubar,
                 int _cubar,
                 double _sim_bar,
                 double _sim_ubar,
                 double _weight_bar,
                 int _intersect_bar,
                 std::string _sim_method) 
      : paracel::paralg(hosts_dct_str, comm, _output),
        input(_input),
        output(_output),
        fmt(_fmt),
        topk(_topk),
        rbar(_rbar),
        cbar(_cbar),
        rubar(_rubar),
        cubar(_cubar),
        sim_bar(_sim_bar),
        sim_ubar(_sim_ubar),
        weight_bar(_weight_bar),
        intersect_bar(_intersect_bar),
        sim_method(_sim_method) {
    if(!(fmt == "fsv" || fmt == "sfv")) {
      std::cout << "format parameter is invalid." << std::endl;
      exit(1);
    }
  }

  virtual ~sim_sparse_row() {}

  void solve() {
    auto local_parser = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    auto local_parser2 = [] (const std::string & line) {
      auto tmp = paracel::str_split(line, ',');
      std::swap(tmp[0], tmp[1]);
      return tmp;
    }; 
    auto f_parser = fmt == "fsv" ? paracel::gen_parser(local_parser) : paracel::gen_parser(local_parser2);
    paracel_load_as_graph(G, input, f_parser, "fmap");
    normalize();
    if(get_worker_id() == 0) std::cout << "normalization finished." << std::endl;
    paracel_write("G_" + std::to_string(get_worker_id()), G);
    paracel_sync();
    if(get_worker_id() == 0) std::cout << "init done" << std::endl;
    auto seq = gen_seq(get_worker_id());
    for(size_t i = 0; i < seq.size(); ++i) {
      if(i == get_worker_id()) {
        cal_sim(G);
        continue;
      }
      paracel_read<paracel::bigraph<node_t> >("G_" + std::to_string(i), G_outside);
      cal_sim(G_outside);
      if(get_worker_id() == 0) std::cout << i << std::endl;
    }
  }
  
  void dump_result() {
    std::unordered_map<node_t, std::vector<std::pair<node_t, double> > > sim_res;
    for(auto & kv : sim_result) {
      sim_res[kv.first].resize(0);
      while(!kv.second.empty()) {
        sim_res[kv.first].push_back(std::make_pair(kv.second.top().first,
                                                   kv.second.top().second));
        kv.second.pop();
      }
      std::reverse(sim_res[kv.first].begin(), sim_res[kv.first].end());
    }
    paracel_dump_dict(sim_res, "similarities_");
    paracel_sync();
  }

 private:
  void normalize() {
    auto lambda = [] (std::unordered_map<node_t,
                                    std::unordered_map<node_t, 
                                                  double> >::value_type & it)  {
      double norm = 0.;
      for(auto & kv : it.second) {
        norm += kv.second * kv.second;
      }
      norm = 1. / sqrt(norm);
      for(auto & kv : it.second) {
        kv.second *= norm;
      }
    };
    G.traverse_by_left_vertex(lambda);
  }

  // calculate similarities between G and Gvar
  void cal_sim(paracel::bigraph<node_t> & Gvar) {
    auto G_data = G.get_data();
    auto Gvar_data = Gvar.get_data();
    for(auto & entity : G_data) {
      for(auto & entity_outside : Gvar_data) {
        auto var = entity.first;
        if(var == entity_outside.first) continue;
        double sim = 0.;
        for(auto & column_entity : entity.second) {
          auto it = entity_outside.second.find(column_entity.first);
          if(it != entity_outside.second.end()) {
            sim += (it->second) * (column_entity.second);
          }
        }
        if(sim < sim_bar || sim > sim_ubar) continue;
        sim_result[var].push(std::make_pair(entity_outside.first, sim));
        if((int)sim_result[var].size() > topk) {
          sim_result[var].pop();
        }
      } // for
    } // for
  }

  std::vector<int> gen_seq(size_t N) {
    std::vector<int> seq;
    size_t wsz = get_worker_size();
    for(size_t i = 0; i < wsz; ++i) {
      seq.push_back((N + i) % wsz);
    }
    return seq;
  }

 private:
  std::string input, output, fmt;
  int topk, rbar, cbar, rubar, cubar;
  double sim_bar, sim_ubar, weight_bar, intersect_bar;
  std::string sim_method;
  paracel::bigraph<node_t> G, G_outside;
  std::unordered_map<node_t, min_heap> sim_result;
}; // class sim_sparse_row

} // namespace alg
} // namespace paracel


#endif
