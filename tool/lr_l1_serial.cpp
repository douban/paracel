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
#include <stdexcept>
#include <gflags/gflags.h>
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

  double L_diff(double a, double y) {
    return -y / (1 + exp(a * y));
  }

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
          delta += L_diff(Z[i], Y[i]) * tmp;
        }
      }
      delta *= 1. / nsamples;
      double ita = std::max(-W[j], -delta * 4);
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
    pt->paracel_dump_vector(rW, "lr_l1_weight_", "|");
  }

  void check() {
    double err = 0.;
    for(int i = 0; i < nsamples; ++i) {
      std::vector<double> tmp(X[i].begin(), X[i].end());
      double a = paracel::dot_product(tmp, rW);
      double pred = 1. / (1 + exp(-a));
      std::cout << "predict|observe : " << pred << "|" << Y[i] << std::endl;
      err += log(1 + exp(-a * Y[i]));
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
      double label = std::stod(linev.back());
      if(label == 0.) label = -1.;
      Y.push_back(label);
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
  int kdim = 0;
  int nsamples = 0;
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
  std::string input, output;
  double lambda;
  int rounds;
  try {
    input = pt.check_parse<std::string>("input");
    output = pt.parse<std::string>("output");
    lambda = pt.parse<double>("lambda");
    rounds = pt.parse<int>("rounds");
  } catch (const std::invalid_argument & e) {
    std::cerr << e.what();
    return 1;
  }
  paracel::tool::lasso solver(comm, input, output, lambda, rounds);
  solver.solve();
  solver.dump_result();
  solver.check();
  return 0;
}
