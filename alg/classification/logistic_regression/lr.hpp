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

#ifndef PARACEL_LOGISTIC_REGRESSION_HPP
#define PARACEL_LOGISTIC_REGRESSION_HPP

#include <string>
#include <vector>

#include "ps.hpp"
#include "utils.hpp"

using namespace std;

namespace paracel {
namespace alg {

class logistic_regression: public paracel::paralg {

 public:
  logistic_regression(paracel::Comm,
                      string,
                      string _input,
                      string output,
                      string update_file_name,
                      string update_func_name,
                      string = "ipm",
                      int _rounds = 1,
                      double _alpha = 0.002,
                      double _beta = 0.1,
                      bool _debug = false);

  virtual ~logistic_regression();

  double lr_hypothesis(const vector<double> &);

  void dgd_learning(); // distributed gradient descent learning
  void ipm_learning(); // by default: iterative parameter mixtures learning
  void downpour_learning(); // asynchronous gradient descent learning
  void agd_learning(); // slow asynchronous gradient descent learning

  virtual void solve();

  double calc_loss();
  void dump_result();
  void print(const vector<double> &);
  void test(const std::string &);
  void predict(const std::string &);

 private:
  void local_parser(const vector<string> &, const char);

  void local_parser_pred(const vector<string> &, const char);

 private:
  string input;
  string update_file, update_func;
  std::string learning_method;
  int worker_id;
  int rounds;
  double alpha, beta;
  bool debug = false;
  vector<vector<double> > samples, pred_samples;
  vector<double> labels;
  vector<double> theta;
  vector<double> loss_error;
  vector<std::pair<vector<double>, double> > predv;
  int kdim; // not contain 1
}; 

} // namespace alg
} // namespace paracel

#endif
