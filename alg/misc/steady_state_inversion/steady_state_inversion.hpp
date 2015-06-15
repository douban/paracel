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

#ifndef FILE_15b06793_4d7d_d6cc_2863_748fad5baa28_HPP
#define FILE_15b06793_4d7d_d6cc_2863_748fad5baa28_HPP

#include <cmath>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "ps.hpp"
#include "load.hpp"
#include "graph.hpp"
#include "utils.hpp"

namespace paracel {
namespace alg {

const double epsilon = 0.00001;
const double delta = epsilon / 2.;

class steady_state_inversion : public paracel::paralg {
 public:
  steady_state_inversion(Comm comm,
                         string hosts_dct_str,
                         string _bigraph_input,
                         string _dis_input,
                         string _output,
                         int _ktop) : 
      paracel::paralg(hosts_dct_str, comm, _output),
      bigraph_input(_bigraph_input),
      dis_input(_dis_input),
      ktop(_ktop) {}

  virtual ~steady_state_inversion() {}

  virtual void solve() {}

  void dump_result() {}

 private:
  void learning() {
  }

 private:
  string bigraph_input, dis_input;
  int ktop;
  paracel::bigraph<string> bi_G;
  unordered_map<string, double> score_new, score;
  unordered_map<string, double> pl, pr;
  paralg *pt;
  unordered_map<string, vector<std::pair<string, double> > > transfer_prob_mtx;

}; // class steady_state_inversion

} // namespace alg
} // namespace paracel

#endif
