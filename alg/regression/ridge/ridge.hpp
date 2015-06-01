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

#ifndef FILE_49bad1db_ea7d_9a05_78f3_28202165825f_HPP
#define FILE_49bad1db_ea7d_9a05_78f3_28202165825f_HPP

#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include "ps.hpp"
#include "utils.hpp"

namespace paracel {
namespace alg {

class ridge_regression: public paracel::paralg {

 public:
  ridge_regression(paracel::Comm comm,
                   std::string hosts_dct_str,
                   std::string _input,
                   std::string output,
                   std::string update_fn,
                   std::string update_fcn,
                   int _rounds,
                   double _alpha,
                   double _beta)
      : paracel::paralg(hosts_dct_str, comm, output, _rounds, 0, false),
      input(_input),
      update_file(update_fn),
      update_func(update_fcn),
      worker_id(comm.get_rank()),
      rounds(_rounds),
      alpha(_alpha),
      beta(_beta) { kdim = 0; loss_error.resize(0); }

  virtual ~ridge_regression() {}

  virtual void solve() {
    auto lines = paracel_load(input);
    local_parser(lines);
    paracel_sync();
    ipm_learning();
    paracel_sync();
  }
  
  void ipm_learning() {
    int data_sz = samples.size();
    int data_dim = samples[0].size();
    theta = paracel::random_double_list(data_dim);
    paracel_write("theta", theta);
    std::vector<int> idx;
    for(int i = 0; i < data_sz; ++i) {
      idx.push_back(i);
    }
    paracel_register_bupdate(update_file,
                             update_func);
    double coff2 = 2. * beta * alpha;
    double wgt = 1. / get_worker_size();
    std::vector<double> delta(data_dim);

    unsigned time_seed = std::chrono::system_clock::now().time_since_epoch().count();
    for(int rd = 0; rd < rounds; ++rd) {
      std::shuffle(idx.begin(), idx.end(), std::default_random_engine(time_seed));
      theta = paracel_read<std::vector<double> >("theta");
      std::vector<double> theta_old(theta);
      
      for(auto sample_id : idx) {
        double coff1 = alpha * (labels[sample_id] - 
                                paracel::dot_product(samples[sample_id], theta));
        for(int i = 0; i < data_dim; ++i) {
          double t = coff1 * samples[sample_id][i] - coff2 * theta[i];
          theta[i] += t;
        }
      }
      for(int i = 0; i < data_dim; ++i) {
        delta[i] = wgt * (theta[i] - theta_old[i]);
      }
      paracel_sync();
      paracel_bupdate("theta", delta);
      paracel_sync();
      std::cout << "worker" << worker_id << " finished round: " << rd << std::endl;
    } // rounds
    theta = paracel_read<std::vector<double> >("theta");
  }

  double calc_loss() {
    double loss = 0.;
    for(size_t i = 0; i < samples.size(); ++i) {
      double h = paracel::dot_product(samples[i], theta) - labels[i];
      loss += h * h;
    }
    auto worker_comm = get_comm();
    worker_comm.allreduce(loss);
    int sz = samples.size();
    worker_comm.allreduce(sz);
    return loss / static_cast<double>(sz);
  }

  void dump_result() {
    if(worker_id == 0) {
      paracel_dump_vector(theta, "ridge_theta_", "|");
    }
    paracel_dump_vector(predv, "pred_v_", "\n");
  }

  void test(const std::string & test_fn) {
    auto lines = paracel_load(test_fn);
    local_parser(lines);
    std::cout << "loss in test dataset is: " 
        << calc_loss() << std::endl;
    paracel_sync();
  }

  void predict(const std::string & pred_fn) {
    auto lines = paracel_load(pred_fn);
    pred_samples.resize(0);
    local_parser_pred(lines);
    predv.resize(0);
    for(size_t i = 0; i < pred_samples.size(); ++i) {
      predv.push_back(std::make_pair(pred_samples[i], paracel::dot_product(pred_samples[i], theta)));
    }
    paracel_sync();
  }

 private:
  void local_parser(const std::vector<std::string> & linelst,
                    const char sep = ',') {
    samples.resize(0);
    labels.resize(0);
    for(auto & line : linelst) {
      std::vector<double> tmp;
      auto linev = paracel::str_split(line, sep);
      tmp.push_back(1.);
      for(size_t i = 0; i < linev.size() - 1; ++i) {
        tmp.push_back(std::stod(linev[i]));
      }
      samples.push_back(tmp);
      labels.push_back(std::stod(linev.back()));
      if(kdim == 0) {
        kdim = samples[0].size() - 1;
      }
    }
  }

  void local_parser_pred(const std::vector<std::string> & linelst,
                         const char sep = ',') {
  
    for(auto & line : linelst) {
      std::vector<double> tmp;
      auto linev = paracel::str_split(line, sep);
      tmp.push_back(1.);
      for(int i = 0; i < kdim; ++i) {
        tmp.push_back(std::stod(linev[i]));
      }
      pred_samples.push_back(tmp);
    }
  }

 private:
  std::string input;
  std::string update_file, update_func;
  int worker_id;
  int rounds;
  double alpha, beta;
  std::vector<std::vector<double> > samples, pred_samples;
  std::vector<double> labels;
  std::vector<double> theta;
  std::vector<double> loss_error;
  std::vector<std::pair<std::vector<double>, double> > predv;
  int kdim;
}; // class ridge_regression

} // namespacec alg
} // namespace paracel

#endif
