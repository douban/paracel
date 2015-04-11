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

#ifndef FILE_c8a6be00_163f_6ae8_c447_47f0d8483425_HPP
#define FILE_c8a6be00_163f_6ae8_c447_47f0d8483425_HPP

#include <string>
#include <unordered_map>
#include "ps.hpp"
#include "load.hpp"
#include "graph.hpp"

namespace paracel {
namespace alg {

#define SENTINEL 2147483648

using node_t = paracel::default_id_type;

class pagerank : public paracel::paralg {
 
 public:
  pagerank(paracel::Comm comm, 
           std::string hosts_dct_str,
           std::string _input,
           std::string _output,
           std::string handle_fn,
           std::string update_fcn,
           std::string filter_fcn,
           int _rounds = 1,
           double df = 0.85) :
    paracel::paralg(hosts_dct_str, comm, _output, _rounds),
    input(_input),
    handle_file(handle_fn),
    update_function(update_fcn),
    filter_function(filter_fcn),
    rounds(_rounds), 
    damping_factor(df) {}

  virtual ~pagerank() {}

  void init_paras() {
    auto local_parser = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    auto f_parser = paracel::gen_parser(local_parser);
    paracel_load_as_graph(local_graph, input, f_parser, "fmap");
    if(get_worker_id() == 0) std::cout << "load done" << std::endl;

    auto cnt_lambda = [&] (const node_t & a,
                           const node_t & b,
                           double c) {
      if(!kvmap.count(a)) {
        kvmap[a] = 1.;
      } else {
        kvmap[a] += 1.;
      }
    };
    local_graph.traverse(cnt_lambda);
    
    // make sure there are no same pieces
    // generate kv + local combine
    auto kvinit_lambda = [&] (const node_t & a,
                              const node_t & b,
                              double c) {
      klstmap[b].push_back(std::make_pair(a, kvmap[a]));
    };
    local_graph.traverse(kvinit_lambda);
    if(get_worker_id() == 0) std::cout << "stat done" << std::endl;

    // init push to construct global connect info
    std::unordered_map<std::string,
        std::vector<std::pair<node_t, double> > > klstmap_tmp;
    for(auto & kv : klstmap) {
      if(kv.first == SENTINEL) continue; // little tricky here
      klstmap_tmp[paracel::cvt(kv.first) + "_links"] = kv.second;
    }
    paracel_bupdate_multi(klstmap_tmp,
                          handle_file,
                          update_function);
    if(get_worker_id() == 0) std::cout << "first bupdate done" << std::endl;
    paracel_sync();

    // read connect info only once
    klstmap.clear();
    for(auto & kv : kvmap) {
      // notice: limit memory here
      paracel_read<std::vector<std::pair<node_t, double> > >
          (paracel::cvt(kv.first) + "_links",
           klstmap[kv.first]);
    }
    if(get_worker_id() == 0) std::cout << "first read done" << std::endl;

    // reuse kvmap to store pr
    // init pr with 1. / total_node_sz
    auto worker_comm = get_comm();
    long node_sz = kvmap.size();
    worker_comm.allreduce(node_sz);
    double init_val = 1. / node_sz;
    std::unordered_map<std::string, double> tmp;
    for(auto & kv : kvmap) {
      kvmap[kv.first] = init_val; 
      tmp[paracel::cvt(kv.first) + "_pr"] = init_val;
    }
    paracel_write_multi(tmp);
    paracel_sync();
  }

  void learning() {
    // first read
    std::unordered_map<node_t, double> kvmap_stale;
    for(auto & kv : klstmap) {
      for(auto & kkv : kv.second) {
        if(!kvmap_stale.count(kkv.first)) {
          //if(klstmap[kkv.first].size() == 0) continue;
          kvmap_stale[kkv.first] = paracel_read<double>(paracel::cvt(kkv.first) + "_pr");
        }
      }
    }

    paracel_sync();

    for(int rd = 0; rd < rounds; ++rd) {
      if(get_worker_id() == 0) std::cout << rd << std::endl;
      // pull
      paracel::list_type<paracel::str_type> keys;
      for(auto & kv : kvmap_stale) {
        keys.push_back(paracel::cvt(kv.first) + "_pr");
      }
      auto result_tmp = paracel_read_multi<double>(keys);
      keys.resize(0);
      int cnt = 0;
      for(auto & kv : kvmap_stale) {
        kvmap_stale[kv.first] = result_tmp[cnt];
        cnt ++;
      }
      result_tmp.resize(0);

      // map
      for(auto & kv : klstmap) {
        double sigma = 0.;
        for(auto & item : kv.second) {
          sigma += (kvmap_stale[item.first] / item.second);
        }
        kvmap[kv.first] = (1. - damping_factor) + damping_factor * sigma;
      }
      paracel_sync();

      std::unordered_map<std::string, double> kvmap_dct;
      // reduce
      for(auto & kv : kvmap) {
        kvmap_dct[paracel::cvt(kv.first) + "_pr"] = kv.second;
      }
      paracel_write_multi(kvmap_dct);
      paracel_sync();
    }
    // last pull all
    auto kvmap_tmp = paracel_read_special<double>(handle_file,
                                                  filter_function);
    auto tear_lambda = [] (const std::string & str) {
      auto pos = str.find('_');
      return str.substr(0, pos);
    };
    for(auto & kv : kvmap_tmp) {
      std::string tmp = tear_lambda(kv.first);
      kvmap[paracel::cvt(tmp)] = kv.second;
    }
  }

  void dump_result() {
    std::unordered_map<std::string, double> kvmap_dump;
    for(auto & kv : kvmap) {
      kvmap_dump[paracel::cvt(kv.first)] = kv.second;
    }
    kvmap.clear();
    if(get_worker_id() == 0) {
      paracel_dump_dict(kvmap_dump, "pagerank_");
    }
  }

  void solve() {
    init_paras();
    paracel_sync();
    learning();
  }

 private:
  std::string input;
  std::string handle_file;
  std::string update_function;
  std::string filter_function;
  int rounds;
  double damping_factor;
  paracel::digraph<node_t> local_graph;
  std::unordered_map<node_t, double> kvmap;
  std::unordered_map<
      node_t, 
      std::vector<std::pair<node_t, double> > 
          > klstmap;
}; // class pagerank

} // namespace alg
} // namespace paracel

#endif
