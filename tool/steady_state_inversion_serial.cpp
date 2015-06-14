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

#include <cmath>
#include <string>
#include <iostream>
#include <unordered_map>

#include <google/gflags.h>

#include "ps.hpp"
#include "load.hpp"
#include "graph.hpp"
#include "utils.hpp"

using std::string;
using std::vector;
using std::unordered_map;
using paracel::paralg;
using paracel::Comm;
using paracel::str_split;

namespace paracel {
namespace tool {

const double epsilon = 0.00001;
const double delta = epsilon / 2.;

class steady_state_inversion {
 
 public:
  steady_state_inversion(Comm comm,
                         string _bigraph_input,
                         string _dis_input,
                         string _output) 
      : bigraph_input(_bigraph_input), 
        dis_input(_dis_input) {
    pt = new paralg(comm, _output);
  }

  ~steady_state_inversion() { delete pt; }

  bool check() { return true; }
  bool check_consistency() { return true; }

  void solve() {
    // TODO
    if(!check()) { ; }

    auto local_parser = [] (const string & line) {
      return paracel::str_split(line, ',');
    };
    auto f_parser = paracel::gen_parser(local_parser);
    pt->paracel_load_as_graph(bi_G, bigraph_input, f_parser);

    // load and init score, pl, pr
    auto linelst = pt->paracel_loadall(dis_input);
    for(auto & line : linelst) {
      auto lst = paracel::str_split(line, ',');
      string key = lst[0];
      double value = std::stod(lst[1]);
      score[key] = value;
      if(paracel::endswith(lst[0], "L")) {
        pl[key] = value;
      } else {
        // paracel::endswith(lst[0], 'R')
        pr[key] = value;
      }
    }

    // train model
    learning();
    for(auto & kv : score) {
      if(paracel::endswith(kv.first, "R")) {
        score_new[kv.first] = kv.second;
      }
    }
  }

  void dump_result() {
    pt->paracel_dump_dict(score_new, "score_");
  }

 private:
  double trans_prob(const string & i, const string & j) {
    if(!bi_G.is_connected(i, j)) {
      return 0.;
    }
    double v = bi_G.get_wgt(i, j) * score[j];
    double sum = 0.;
    auto traverse_lambda = 
        [&] (const string & i,
             const string & jj,
             double w) {
      sum += bi_G.get_wgt(i, jj) * score[jj];
    };
    bi_G.traverse(i, traverse_lambda);
    return v / sum;
  }

  double q(string rnode) {
    double r = 0.;
    for(auto & kv : pl) {
      auto node = kv.first;
      r += kv.second * trans_prob(node, rnode);
    }
    return r;
  }
  
  double newton_root(const unordered_map<string, std::pair<double, double> > & AC,
                     double L,
                     string j) {
    double x = score[j]; // start with sj(t-1)
    for(int it = 0; it < 100; ++it) {
      double f = 0, fp = 0.;
      for(auto & kv : AC) {
        double ai = kv.second.first;
        double ci = kv.second.second;
        double coff1 = pl[kv.first] * ai; // pi * ai
        double coff2 = ai * x + ci;
        f += coff1 * x * (1. / coff2);
        fp += coff1 * ci * (1 / pow(coff2, 2.));
      }
      f = f - L;
      if(fabs(f) <= 1e-10 || fp == 0) { break; }
      x = x - f / fp;
      if(x != x) { std::cout << "overflow" << std::endl; break; }
    }
    return x;
  }

  double q_solve(string rnode) {
    unordered_map<string, std::pair<double, double> > AC;
    for(auto & kv : pl) {
      auto node = kv.first;
      if(!bi_G.is_connected(node, rnode)) { continue; }
      double sum = 0.;
      auto temp_lambda = [&] (const string & f,
                              const string & s,
                              const double v) {
        if(s != rnode) {
          sum += bi_G.get_wgt(f, s) * score[s];
        }
      };
      bi_G.traverse(node, temp_lambda);
      AC[node] = std::make_pair(bi_G.get_wgt(node, rnode),
                                sum);
    }
    return newton_root(AC, pr[rnode] * (1 - delta), rnode);
  }
  
  void learning() {
    int t = 0;
    int outlier = 0;
    do {
      t += 1;
      outlier = 0;
      for(auto & kv : pr) {
        string j = kv.first;
        double pj = kv.second;
        double left_v = q(j);
        if(left_v == 0.) { continue; }
        if(left_v < (pj * (1 - epsilon))) {
          std::cout << j << " qj " << q(j) << "|" << pj * (1 - epsilon) << std::endl;
          outlier += 1;
          std::cout << j << " before: " << score[j] << std::endl;
          // solve x
          score_new[j] = q_solve(j);
          std::cout << j << " after: " << score_new[j] << std::endl;
        }
      } // for
      for(auto & kv : score_new) {
        score[kv.first] = kv.second;
      }
      std::cout << "it: " << t << " with outliers:" << outlier << std::endl;
      std::cout << std::endl;
    } while(outlier && t < 100); // do-while
  }

 private:
  string bigraph_input, dis_input;
  paracel::bigraph<string> bi_G;
  unordered_map<string, double> score_new, score;
  unordered_map<string, double> pl, pr;
  paralg *pt;
}; // class steady_state_inversion

} // namespace tool 
} // namespace paracel

DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  
  google::SetUsageMessage("[options]\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);

  paracel::json_parser pt(FLAGS_cfg_file);
  string bigraph_input = pt.parse<string>("bigraph_input");
  string dis_input = pt.parse<string>("distribution_input");
  string output = pt.parse<string>("output");

  paracel::tool::steady_state_inversion solver(comm, bigraph_input, dis_input, output);
  solver.solve();
  solver.dump_result();
  
  return 0;
}
