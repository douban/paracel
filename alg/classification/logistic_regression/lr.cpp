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

/*
 * In this implementation, theta is only sent to one parameter server(reducer).
 * You can split theta into multi-servers when theta is very high-dimensional(In this case
 *  you'd better use l1-regularization lr instead) or for optimization purpose.
 * We also omit the details of learning-rate adjustments.
 */

#include <math.h>

#include <string>
#include <algorithm>
#include <iostream>

#include "lr.hpp"
#include "ps.hpp"
#include "utils.hpp"

using std::vector;
using std::string;

namespace paracel {
namespace alg {

logistic_regression::logistic_regression(paracel::Comm comm,
                                         string hosts_dct_str,
                                         string _input,
                                         string output,
                                         string update_fn,
                                         string update_fcn,
                                         string method,
                                         int _rounds,
                                         double _alpha,
                                         double _beta,
                                         bool _debug) 
    : paracel::paralg(hosts_dct_str,
                      comm,
                      output,
                      _rounds,
                      0, // limit_s
                      false), // ssp_switch
	input(_input),
  update_file(update_fn),
  update_func(update_fcn),
	learning_method(method),
	worker_id(comm.get_rank()),
	rounds(_rounds), 
	alpha(_alpha),
	beta(_beta),
	debug(_debug) { kdim = 0; loss_error.resize(0); }

logistic_regression::~logistic_regression() {}

double logistic_regression::lr_hypothesis(const vector<double> & v) {
  double dp = paracel::dot_product(v, theta);
  return 1. / (1. + exp(-dp));
}

// initialize
// training data format: feature_1,feature_2,...,feature_k,label
void logistic_regression::local_parser(const vector<string> & linelst,
                                       const char sep = ',') {
  samples.resize(0);
  labels.resize(0);
  for(auto & line : linelst) {
    vector<double> tmp;
    auto linev = paracel::str_split(line, sep); 
    tmp.push_back(1.);
    for(size_t i = 0; i < linev.size() - 1; ++i) {
      tmp.push_back(std::stod(linev[i]));
    }
    samples.push_back(tmp);
    labels.push_back(std::stod(linev.back()));
  }
  if(kdim == 0) {
    kdim = samples[0].size() - 1;
  }
}

// predict data format: feature_1,feature_2,..,feature_k,...
void logistic_regression::local_parser_pred(const vector<string> & linelst,
                                            const char sep = ',') {
  samples.resize(0);
  for(auto & line : linelst) {
    vector<double> tmp;
    auto linev = paracel::str_split(line, sep); 
    tmp.push_back(1.);
    for(int i = 0; i < kdim; ++i) {
      tmp.push_back(std::stod(linev[i]));
    }
    samples.push_back(tmp);
  }
}

void logistic_regression::dgd_learning() {
  int data_sz = samples.size();
  int data_dim = samples[0].size();
  theta = paracel::random_double_list(data_dim); 
  paracel_write("theta", theta); // first push
  vector<int> idx;
  for(int i = 0; i < data_sz; ++i) {
    idx.push_back(i);
  }
  paracel_register_bupdate(update_file, update_func);
  double coff2 = 2. * beta * alpha;
  vector<double> delta(data_dim); 

  // train loop
  for(int rd = 0; rd < rounds; ++rd) {
    for(int i = 0; i < data_dim; ++i) {
      delta[i] = 0.;
    }
    std::random_shuffle(idx.begin(), idx.end()); 
    theta = paracel_read<vector<double> >("theta"); 
    
    // traverse data
    for(auto sample_id : idx) {
      double grad = labels[sample_id] - lr_hypothesis(samples[sample_id]); 
      double coff1 = alpha * grad;
      for(int i = 0; i < data_dim; ++i) {
        double t = coff1 * samples[sample_id][i] - coff2 * theta[i];
        delta[i] += t;
      }
      if(debug) {
        loss_error.push_back(calc_loss());
      }
    } // traverse
    sync(); // sync for map
    paracel_bupdate("theta", delta); // update with delta
    sync(); // sync for reduce
    
    std::cout << "worker" << get_worker_id() 
        << " finished round: " << rd 
        << std::endl;
  } // train loop
  theta = paracel_read<vector<double> >("theta"); // last pull
}

void logistic_regression::ipm_learning() {
  int data_sz = samples.size();
  int data_dim = samples[0].size();
  theta = paracel::random_double_list(data_dim); 
  paracel_write("theta", theta); // first push
  vector<int> idx;
  for(int i = 0; i < data_sz; ++i) { 
    idx.push_back(i);
  }
  paracel_register_bupdate(update_file,
                           update_func);
  double coff2 = 2. * beta * alpha;
  double wgt = 1. / get_worker_size();
  vector<double> delta(data_dim);

  // train loop
  for(int rd = 0; rd < rounds; ++rd) {
    std::random_shuffle(idx.begin(), idx.end()); 
    theta = paracel_read<vector<double> >("theta"); 
    vector<double> theta_old(theta);

    // traverse data
    for(auto sample_id : idx) {
      double coff1 = alpha * (labels[sample_id] - lr_hypothesis(samples[sample_id])); 
      for(int i = 0; i < data_dim; ++i) {
        double t = coff1 * samples[sample_id][i] - coff2 * theta[i];
        theta[i] += t;
      }
      if(debug) {
        loss_error.push_back(calc_loss());
      }
    } // traverse
    
    for(int i = 0; i < data_dim; ++i) {
      delta[i] = wgt * (theta[i] - theta_old[i]);
    }
    sync(); // sync for map
    paracel_bupdate("theta", delta); // update with delta
    sync(); // sync for reduce
    
    std::cout << "worker" << get_worker_id() 
        << " finished round: " << rd 
        << std::endl;
  } // rounds
  theta = paracel_read<vector<double> >("theta"); // last pull
}

void logistic_regression::downpour_learning() {
  int data_sz = samples.size();
  int data_dim = samples[0].size();
  int cnt = 0;
  int read_batch = data_sz / 800, update_batch = data_sz / 1000;
  if(read_batch == 0) read_batch = 10;
  if(update_batch == 0) update_batch = 10;
  theta = paracel::random_double_list(data_dim); 
  paracel_write("theta", theta); // init push
  vector<int> idx;
  for(int i = 0; i < data_sz; ++i) { 
    idx.push_back(i);
  }
  paracel_register_bupdate(update_file, update_func);
  double coff2 = 2. * beta * alpha;
  vector<double> delta(data_dim);

  // train loop
  for(int rd = 0; rd < rounds; ++rd) {
    std::random_shuffle(idx.begin(), idx.end()); 
    theta = paracel_read<vector<double> >("theta"); 
    vector<double> theta_old(theta);
    
    // traverse data
    cnt = 0;
    for(auto sample_id : idx) {
      if( (cnt % read_batch == 0) || (cnt == (int)idx.size() - 1) ) { 
        theta = paracel_read<vector<double> >("theta");
        theta_old = theta;
      }
      double coff1 = alpha * (labels[sample_id] - lr_hypothesis(samples[sample_id])); 
      for(int i = 0; i < data_dim; ++i) {
        double t = coff1 * samples[sample_id][i] - coff2 * theta[i];
        theta[i] += t;
      }
      if(debug) {
        loss_error.push_back(calc_loss());
      }
      if( (cnt % update_batch == 0) || (cnt == (int)idx.size() - 1) ) {
        for(int i = 0; i < data_dim; ++i) {
          delta[i] = theta[i] - theta_old[i];
        }
        paracel_bupdate("theta", delta);
      }
      cnt += 1;
    } // traverse
    sync();

    std::cout << "worker" << get_worker_id() 
        << " finished round: " << rd 
        << std::endl;
  } // rounds
  theta = paracel_read<vector<double> >("theta"); // last pull
}

void logistic_regression::agd_learning() {
  int data_sz = samples.size();
  int data_dim = samples[0].size();
  theta = paracel::random_double_list(data_dim); 
  paracel_write("theta", theta); // first push
  vector<int> idx;
  for(int i = 0; i < data_sz; ++i) { 
    idx.push_back(i);
  }
  paracel_register_bupdate(update_file, update_func);
  double coff2 = 2. * beta * alpha;
  vector<double> delta(data_dim);

  // train loop
  for(int rd = 0; rd < rounds; ++rd) {
    std::random_shuffle(idx.begin(), idx.end()); 
    theta = paracel_read<vector<double> >("theta"); 
    vector<double> theta_old(theta);

    // traverse data
    for(auto sample_id : idx) {
      theta = paracel_read<vector<double> >("theta"); 
      theta_old = theta;
      double coff1 = alpha * (labels[sample_id] - lr_hypothesis(samples[sample_id])); 
      for(int i = 0; i < data_dim; ++i) {
        double t = coff1 * samples[sample_id][i] - coff2 * theta[i];
        theta[i] += t;
      }
      if(debug) {
        loss_error.push_back(calc_loss());
      }
      for(int i = 0; i < data_dim; ++i) {
        delta[i] = theta[i] - theta_old[i];
      }
      
      paracel_bupdate("theta", delta); // you could push a batch of delta into a queue to optimize
    } // traverse

  } // rounds
  theta = paracel_read<vector<double> >("theta"); // last pull
}

void logistic_regression::solve() {
  
  auto lines = paracel_load(input);
  local_parser(lines);
  sync();

  if(learning_method == "dgd") {
    dgd_learning();
  } else if(learning_method == "ipm") {
    ipm_learning();
  } else if(learning_method == "downpour") {
    downpour_learning();
  } else if(learning_method == "agd") {
    agd_learning();
  } else {
    ERROR_ABORT("method do not support");
  }
  sync();
}

double logistic_regression::calc_loss() {
  double loss = 0.;
  for(size_t i = 0; i < samples.size(); ++i) {
    double h = lr_hypothesis(samples[i]);
    if(labels[i] == 1.) {
      loss += -log(h);
    } else {
      loss += -log(1-h);
    }
  }
  auto worker_comm = get_comm();
  worker_comm.allreduce(loss);
  int sz = samples.size();
  worker_comm.allreduce(sz);
  return loss / static_cast<double>(sz);
}

void logistic_regression::dump_result() {
  if(worker_id == 0) {
    paracel_dump_vector(theta, "lr_theta_", "|");
    if(debug) {
      paracel_dump_vector(loss_error, "lr_loss_error_", "\n");
    }
  }
  paracel_dump_vector(predv, "pred_v_", "\n");
}

void logistic_regression::test(const std::string & test_fn) {
  auto lines = paracel_load(test_fn);
  local_parser(lines);
  std::cout << "loss in test dataset is: " 
      << calc_loss() << std::endl;
}

void logistic_regression::predict(const std::string & pred_fn) {
  auto lines = paracel_load(pred_fn);
  local_parser_pred(lines);
  for(size_t i = 0; i < samples.size(); ++i) {
    predv.push_back(std::make_pair(samples[i], lr_hypothesis(samples[i])));
  }
  sync();
}

} // namespace alg
} // namespace paracel
