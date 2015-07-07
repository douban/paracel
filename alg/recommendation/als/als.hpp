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

/**
 * A subprocedure of training user/item factor, just one iteration.
 *
 */

#ifndef FILE_ff90b44a_7b01_0d57_e654_70570f34db29_HPP
#define FILE_ff90b44a_7b01_0d57_e654_70570f34db29_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

#include <eigen3/Eigen/Dense>

#include "ps.hpp"
#include "graph.hpp"
#include "load.hpp"
#include "utils.hpp"

namespace paracel {
namespace alg {

//using node_t = std::string;
using node_t = paracel::default_id_type;

class alternating_least_square_standard : public paracel::paralg {
 
 public:
  alternating_least_square_standard(paracel::Comm comm,
                                    std::string hosts_dct_str,
                                    std::string _rating_input,
                                    std::string _factor_input,
                                    std::string _output,
                                    std::string _pattern,
                                    double _lambda)
  : paracel::paralg(hosts_dct_str, comm, _output),
    rating_input(_rating_input),
    factor_input(_factor_input),
    pattern(_pattern),
    lambda(_lambda) {}

  virtual ~alternating_least_square_standard() {}

  virtual void solve() {

    // load rating_graph, partition with pattern
    auto local_parser_rating = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    auto rating_parser_func = paracel::gen_parser(local_parser_rating);
    paracel_load_as_graph(rating_graph,
                          rating_input,
                          rating_parser_func,
                          pattern);

    std::unordered_set<std::string> local_H_set;
    std::vector<double> empty;
    auto traverse_lambda = [&] (const node_t & a, const node_t & b, double c) {
      W[a] = empty;
      local_H_set.insert(paracel::cvt(b));
    };
    rating_graph.traverse(traverse_lambda);
    
    // load H 
    auto local_parser_factor = [&] (const std::vector<std::string> & linelst) {
      for(auto & line : linelst) {
        auto v = paracel::str_split(line, '\t');
        auto fac = paracel::str_split(v[1], '|');
        kdim = fac.size();
        std::vector<double> tmp;
        if(local_H_set.count(v[0])) {
          for(auto & vv : fac) {
            tmp.push_back(std::stod(vv));
          }
          H[paracel::cvt(v[0])] = tmp;
        } // if
      } // for
    };
    paracel_sequential_loadall(factor_input, local_parser_factor);
    
    learning();
  }

  void dump_result() {
    std::unordered_map<std::string, std::vector<double> > dump_W;
    for(auto & kv : W) {
      dump_W[std::to_string(kv.first)] = kv.second;
    }
    paracel_dump_dict(dump_W, "W_");
  }
  
  double cal_rmse() {
    auto worker_comm = get_comm();
    double rmse = 0;
    auto rmse_lambda = [&] (const node_t & uid,
                            const node_t & iid,
                            double rating) {
      double e = rating - estimate(uid, iid);
      rmse += e * e;
    };
    rating_graph.traverse(rmse_lambda);
    worker_comm.allreduce(rmse);
    long long sz_sum = rating_graph.e();
    worker_comm.allreduce(sz_sum);
    return sqrt(rmse / sz_sum);
  }

 private:
  void learning() {
    int cnt = 0;
    for(auto & kv : W) {
      auto uid = kv.first;
      std::vector<double> ai_vec;
      std::vector<std::vector<double> > H_sub_vec;
      auto local_lambda = [&] (const node_t & a,
                               const node_t & b,
                               double v) {
          if(!H.count(b)) { 
            std::cout << a << "," << b << "," << v << std::endl;
            throw std::runtime_error("Data error: rating data and factor data is not consistent.\n"); 
          }
          H_sub_vec.push_back(H[b]);
          ai_vec.push_back(v);
      };
      rating_graph.traverse(uid, local_lambda);
      Eigen::MatrixXd H_sub(H_sub_vec.size(), kdim);
      auto ai = paracel::vec2evec(ai_vec);
      // construct H_sub, ai
      for(size_t i = 0; i < H_sub_vec.size(); ++i) {
        H_sub.row(i) = Eigen::VectorXd::Map(&H_sub_vec[i][0], kdim);
      }
      // solve als by: inv(H_sub.transpose() * H_sub + lambda * I) * H_sub.transpose() * ai
      auto H_sub_T = H_sub.transpose();
      Eigen::MatrixXd I = Eigen::MatrixXd::Identity(kdim, kdim);
      auto T1 = H_sub_T * H_sub + lambda * I;
      auto T2 = H_sub_T * ai;
      W[uid] = paracel::evec2vec(T1.inverse() * T2);
      cnt += 1;
    } // for
  }

  inline double estimate(const node_t & uid,
                         const node_t & iid) {
    return paracel::dot_product(W[uid], H[iid]);
  }

 private:
  std::string rating_input, factor_input;
  std::string pattern = "fmap"; // fmap(to train user factor) and smap(to train item factor)
  double lambda;
  int kdim = 100;
  paracel::bigraph<node_t> rating_graph;
  std::unordered_map<node_t, std::vector<double> > W, H;
}; // class alternating_least_square_standard


class alternating_least_square_validate : public paracel::paralg {
 
 public:
  alternating_least_square_validate(paracel::Comm comm,
                                    std::string hosts_dct_str,
                                    std::string _rating_input,
                                    std::string _factor_input,
                                    std::string _validate_input,
                                    std::string _output,
                                    std::string _pattern)
  : paracel::paralg(hosts_dct_str, comm, _output),
    rating_input(_rating_input),
    factor_input(_factor_input),
    validate_input(_validate_input),
    pattern(_pattern) {}

  virtual ~alternating_least_square_validate() {}

  virtual void init() {

    // load rating_graph, partition with pattern
    auto local_parser_rating = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    auto rating_parser_func = paracel::gen_parser(local_parser_rating);
    paracel_load_as_graph(rating_graph,
                          rating_input,
                          rating_parser_func,
                          pattern);

    std::vector<double> empty;
    auto traverse_lambda = [&] (const node_t & a, const node_t & b, double c) {
      W[a] = empty;
    };
    rating_graph.traverse(traverse_lambda);
    
    // load H 
    auto local_parser_factor = [&] (const std::vector<std::string> & linelst) {
      for(auto & line : linelst) {
        auto v = paracel::str_split(line, '\t');
        auto fac = paracel::str_split(v[1], '|');
        kdim = fac.size();
        std::vector<double> tmp;
        for(auto & vv : fac) {
          tmp.push_back(std::stod(vv));
        }
        H[paracel::cvt(v[0])] = tmp;
      } // for
    };
    paracel_sequential_loadall(factor_input, local_parser_factor);
  }

  void dump_result() {
    std::unordered_map<std::string, std::vector<double> > dump_W;
    for(auto & kv : W) {
      dump_W[std::to_string(kv.first)] = kv.second;
    }
    paracel_dump_dict(dump_W, "W_");
  }
  
  double cal_rmse() {
    auto worker_comm = get_comm();
    double rmse = 0;
    auto rmse_lambda = [&] (const node_t & uid,
                            const node_t & iid,
                            double rating) {
      double e = rating - estimate(uid, iid);
      rmse += e * e;
    };
    rating_graph.traverse(rmse_lambda);
    worker_comm.allreduce(rmse);
    long long sz_sum = rating_graph.e();
    worker_comm.allreduce(sz_sum);
    return sqrt(rmse / sz_sum);
  }

  double validate() {
    auto worker_comm = get_comm();
    double rmse = 0.; 
    long long sz = 0;
    auto local_parser = [&] (const std::vector<std::string> & linelst) {
      for(auto & line : linelst) {
        auto v = paracel::str_split(line, ',');
        auto uid = paracel::cvt(v[0]);
        if(W.count(uid)) {
          auto iid = paracel::cvt(v[1]);
          double r = std::stod(v[2]);
          if(H.count(iid)) {
            rmse += pow(r - estimate(uid, iid), 2);
            sz ++;
          }
        }
      }
    };
    paracel_sequential_loadall(validate_input, local_parser);
    worker_comm.allreduce(rmse);
    worker_comm.allreduce(sz);
    return sqrt(rmse / sz);
  }

  void learning(double lambda) {
    int cnt = 0;
    for(auto & kv : W) {
      auto uid = kv.first;
      std::vector<double> ai_vec;
      std::vector<std::vector<double> > H_sub_vec;
      auto local_lambda = [&] (const node_t & a,
                               const node_t & b,
                               double v) {
        if(!H.count(b)) { 
          std::cout << a << "," << b << "," << v << std::endl;
          throw std::runtime_error("Data error: rating data and factor data is not consistent.\n"); 
        }
        H_sub_vec.push_back(H[b]);
        ai_vec.push_back(v);
      };
      rating_graph.traverse(uid, local_lambda);
      Eigen::MatrixXd H_sub(H_sub_vec.size(), kdim);
      auto ai = paracel::vec2evec(ai_vec);
      // construct H_sub, ai
      for(size_t i = 0; i < H_sub_vec.size(); ++i) {
        H_sub.row(i) = Eigen::VectorXd::Map(&H_sub_vec[i][0], kdim);
      }
      // solve als by: inv(H_sub.transpose() * H_sub + lambda * I) * H_sub.transpose() * ai
      auto H_sub_T = H_sub.transpose();
      Eigen::MatrixXd I = Eigen::MatrixXd::Identity(kdim, kdim);
      auto T1 = H_sub_T * H_sub + lambda * I;
      auto T2 = H_sub_T * ai;
      W[uid] = paracel::evec2vec(T1.inverse() * T2);
      cnt += 1;
    } // for
  }

 private:
  inline double estimate(const node_t & uid,
                         const node_t & iid) {
    return paracel::dot_product(W[uid], H[iid]);
  }

 private:
  std::string rating_input, factor_input;
  std::string validate_input;
  std::string pattern = "fmap"; // fmap(to train user factor) and smap(to train item factor)
  int kdim = 100;
  paracel::bigraph<node_t> rating_graph;
  std::unordered_map<node_t, std::vector<double> > W, H;
}; // class alternating_least_square_validate

} // namespace alg
} // namespace paracel

#endif
