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

#ifndef FILE_362896bd_c47f_e4d7_ff6e_24dd216859b1_HPP
#define FILE_362896bd_c47f_e4d7_ff6e_24dd216859b1_HPP

#include <iostream>
#include <string>
#include "ps.hpp"
#include "graph.hpp"
#include "paracel_types.hpp"

namespace paracel {
namespace alg {

using node_t = paracel::default_id_type;

class adjust_ktop_s : public paracel::paralg {

 public:
  adjust_ktop_s(paracel::Comm comm,
                std::string hosts_dct_str,
                std::string _rating_input,
                std::string _sim_input,
                std::string _output) : 
      paracel::paralg(hosts_dct_str, comm, _output),
      rating_input(_rating_input),
      sim_input(_sim_input) {}

  virtual ~adjust_ktop_s() {}

  virtual void solve() {

    // load sim_G, model partition
    auto local_parser = [] (const std::string & line) {
      auto tmp = paracel::str_split(line, '\t');
      auto adj = paracel::str_split(tmp[1], '|');
      std::vector<std::string> stuff = {tmp[0]};
      stuff.insert(stuff.end(), adj.begin(), adj.end());
      return stuff;
    };
    auto parser_func = paracel::gen_parser(local_parser);
    paracel_load_as_graph(sim_G,
                          sim_input,
                          parser_func,
                          "fset");

    // load rating_G, data partition
    auto local_parser_rating = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    auto rating_parser_func = paracel::gen_parser(local_parser_rating);
    paracel_load_as_graph(rating_G,
                          rating_input,
                          rating_parser_func,
                          "fmap");
    //load_check();
    cal_low_peak();

  }

 private:
  double residual(int k);

  int binary_search(const node_t & node) {
    int ktop = 0;
    return ktop;
  }

  void cal_low_peak() {
    auto uid_set = sim_G.left_vertex_set();
    for(auto & uid : uid_set) {
      binary_search(uid);
    }
  }

  void load_check() {
    if(get_worker_id() == 0) {
      std::cout << sim_G.v() << "|" << rating_G.v() << std::endl;
      auto sset = rating_G.left_vertex_set();
      auto print_lambda = [] (const node_t & a,
                              const node_t & b,
                              double v) {
        std::cout << a << " | " << b << " | " << v << std::endl;
      };
      sim_G.traverse(print_lambda);
      std::cout << "##################" << std::endl;
      rating_G.traverse(print_lambda);
      auto check_lambda = [&] (const node_t & a,
                               const node_t & b,
                               double v) {
        if(!sset.count(a)) {
          std::cout << a << " is a bug" << std::endl;
        }
      };
      sim_G.traverse(check_lambda);
    }
  }

  void dump_result() {}

 private:
  std::string rating_input;
  std::string sim_input;
  paracel::bigraph<node_t> sim_G;
  paracel::bigraph<node_t> rating_G;
  paracel::dict_type<node_t, int> ktop_dict;

}; // class adjust_ktop_s

} // namespace alg
} // namespace paracel

#endif
