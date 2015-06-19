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

#include <math.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>

#include <google/gflags.h>

#include "ps.hpp"
#include "utils.hpp"

using std::string;
using std::vector;
using std::random_shuffle;
using paracel::paralg;
using paracel::Comm;
using paracel::str_split;
using paracel::random_double_list;
using paracel::dot_product;

namespace paracel {
namespace tool {

class logistic_regression {

public:  
  logistic_regression(Comm comm,
  	std::string _input, 
  	std::string _output,
    size_t _rounds = 1,
    double _alpha = 0.001,
    double _beta = 0.001,
    bool _debug = true) : 
      input(_input),
      output(_output),
      rounds(_rounds),
      alpha(_alpha),
      beta(_beta),
      debug(_debug) {
    pt = new paralg(comm, output, rounds);
  }
  
  ~logistic_regression() {
    delete pt;
  }
 
  double lr_hypothesis(const vector<double> & v) {
    double dp = dot_product(v, theta);
    return 1. / (1. + exp(-dp));
  }

  void local_parser(const vector<string> & linelst,
                    const char sep = ',') {
    samples.resize(0);
    labels.resize(0);
    for(auto & line : linelst) {
      vector<double> tmp;
      auto linev = str_split(line, sep);
      tmp.push_back(1.);
      for(size_t i = 0; i < linev.size() - 1; ++i) {
        tmp.push_back(std::stod(linev[i]));
      }
      samples.push_back(tmp);
      labels.push_back(std::stod(linev.back()));
    }
  }

  void learning() {
    int data_sz = samples.size();
    int data_dim = samples[0].size();
    theta = random_double_list(data_dim);
    vector<int> idx;
    for(int i = 0; i < data_sz; ++i) {
      idx.push_back(i);
    }
    
    double coff2 = 2. * beta * alpha;
    // training loop
    for(int rd = 0; rd < rounds; ++rd) {
      random_shuffle(idx.begin(), idx.end());
      vector<double> delta(data_dim, 0);
      for(auto id : idx) {
        double opt1 = alpha * (labels[id] - lr_hypothesis(samples[id]));
        for(int i = 0; i < data_dim; ++i) {
          double t = opt1 * samples[id][i] - coff2 * theta[i];
          delta[i] += t;
        }
      } // traverse 

      for(int i = 0; i < data_dim; ++i) {
        theta[i] += delta[i];
      }
      if(debug) {
        loss_error.push_back(calc_loss());
      }
      std::cout << "round " << rd << " finished." << std::endl;
    } // training loop 

  }

  void solve() {
    auto lines = pt->paracel_load(input);
    local_parser(lines);
    learning();
  }
  
  double calc_loss() {
    double loss = 0.;
    for(size_t i = 0; i < samples.size(); ++i) {
      double h = lr_hypothesis(samples[i]);
      //std::cout << labels[i] << "|" << h << std::endl;
      if(labels[i] == 1.) {
        loss += -log(h);
      } else {
        loss += -log(1-h);
      }
    }
    return loss / static_cast<double>(samples.size());  
  }

  void dump_result() {
    pt->paracel_dump_vector(theta, "lr_theta_serial_", "|");
    pt->paracel_dump_vector(loss_error, "lr_loss_error_serial_", "\n");
    pt->paracel_dump_vector(predv, "pred_v_serial_", "\n");
  }

  void test(const std::string & test_fn) {
    auto lines = pt->paracel_load(test_fn);
    local_parser(lines);
    std::cout << "loss in test dataset is:" << calc_loss() << std::endl;
  }

  void predict(const std::string & pred_fn) {
    auto lines = pt->paracel_load(pred_fn);
    local_parser(lines);
    for(size_t i = 0; i < samples.size(); ++i) {
      predv.push_back(lr_hypothesis(samples[i]));
    }
  }
   
private:
  string input, output;
  int rounds;
  double alpha, beta;
  vector<vector<double> > samples;
  vector<double> labels, theta;
  paralg *pt;
  bool debug = false;
  vector<double> loss_error;
  vector<double> predv;
};

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
  std::string training_input = pt.check_parse<std::string>("training_input");
  std::string test_input = pt.check_parse<std::string>("test_input");
  std::string predict_input = pt.check_parse<std::string>("predict_input");
  std::string output = pt.parse<std::string>("output");
  std::string update_file = pt.check_parse<std::string>("update_file");
  std::string update_func = pt.parse<std::string>("update_func");
  std::string method = pt.parse<std::string>("method");
  int rounds = pt.parse<int>("rounds");
  double alpha = pt.parse<double>("alpha");
  double beta = pt.parse<double>("beta");
  bool debug = pt.parse<bool>("debug");
  paracel::tool::logistic_regression lr_solver(comm,
                                               training_input,
                                               output,
                                               rounds,
                                               alpha,
                                               beta,
                                               debug);
  lr_solver.solve();
  std::cout << "final loss: " << lr_solver.calc_loss() << std::endl;
  lr_solver.test(test_input);
  lr_solver.predict(predict_input);
  lr_solver.dump_result();
  return 0;
}
