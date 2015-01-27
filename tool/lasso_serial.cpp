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

#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <google/gflags.h>
#include "ps.hpp"
#include "utils.hpp"

namespace paracel {
namespace tool {

class lasso {
 public:
  lasso(Comm comm,
        std::string input,
        std::string output,
        double lambda,
        int rounds) :
      input(input), 
      output(output),
      lambda(lambda),
      rounds(rounds) {
    pt = new paralg(comm, output, rounds);                    
  }

  ~lasso() { delete pt; }

  void learning() {
    for(int rd = 0; rd < rounds; ++rd) {
      int j = random_select();
      int jj = j;
      bool flag = false;
      if(j >= kdim) { flag = true; jj -= kdim; }
      double delta = lambda;
      for(int i = 0; i < nsamples; ++i) {
        if(X[i][jj] != 0) {
	        double tmp = flag ? -X[i][jj] : X[i][jj];
          delta += (Z[i] - Y[i]) * tmp;
        }
      }
      delta *= 1. / nsamples;
      double ita = std::max(-W[j], -delta);
      W[j] += ita;
      for(int i = 0; i < nsamples; ++i) {
        if(X[i][jj] != 0) {
	        double tmp = flag ? -X[i][jj] : X[i][jj];
          Z[i] += ita * tmp;
        }
      }
    } // round loop
  }

  void translating() {
    rW.resize(kdim);
    for(int i = 0; i < kdim; ++i) {
      rW[i] = W[i] - W[kdim + i];
    }
  }

  void dump_result() {
    pt->paracel_dump_vector(rW, "lasso_weight_", "|");
  }

  void check() {
    double err = 0.;
    for(int i = 0; i < nsamples; ++i) {
      std::vector<double> tmp(X[i].begin(), X[i].end());
      double a = paracel::dot_product(tmp, rW);
      std::cout << "predict|observe : " << a << "|" << Y[i] << std::endl;
      err += (a - Y[i]) * (a - Y[i]);
    }
    std::cout << "total error: " << err / nsamples << std::endl;
  }

  void solve() {
    auto lines = pt->paracel_load(input);
    init_data(lines);
    learning();
    translating();
  }

 private:
  void init_data(const std::vector<std::string> & lines) {
    X.resize(0); Y.resize(0); W.resize(0);
    for(auto & sample: lines) {
      std::vector<double> tmp; tmp.push_back(1.);
      auto linev = paracel::str_split(sample, ',');
      for(size_t i = 0; i < linev.size() - 1; ++i) {
        tmp.push_back(std::stod(linev[i]));
      }
      X.push_back(tmp);
      Y.push_back(std::stod(linev.back()));
    }
    kdim = X[0].size();
    nsamples = X.size();
    for(int i = 0; i < 2 * kdim; ++i) {
      W.push_back(0.);
    }
    for(int i = 0; i < nsamples; ++i) {
      Z.push_back(0.);
    }
    for(int i = 0; i < kdim; ++i) {
      rW.push_back(0.);
    }
  }

  int random_select() {
    return std::rand() % (2 * kdim);
  }

 private:
  std::string input, output;
  double lambda;
  int rounds;
  paralg *pt;
  std::vector<std::vector<double> > X;
  std::vector<double> rW, W, Z, Y;
  int kdim;
  int nsamples;
}; // class lasso

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
  std::string input = pt.parse<std::string>("input");
  std::string output = pt.parse<std::string>("output");
  double lambda = pt.parse<double>("lambda");
  int rounds = pt.parse<int>("rounds");
  paracel::tool::lasso solver(comm, input, output, lambda, rounds);
  solver.solve();
  solver.dump_result();
  solver.check();
  return 0;
}
