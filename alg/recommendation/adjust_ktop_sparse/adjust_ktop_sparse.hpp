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

#include <cfloat>
#include <string>
#include <iostream>
#include <algorithm>
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
                std::string _fmt,
                std::string _sim_input,
                int _low_limit,
                std::string _output) : 
      paracel::paralg(hosts_dct_str, comm, _output),
      rating_input(_rating_input),
      fmt(_fmt),
      sim_input(_sim_input),
      low_limit(_low_limit) {}

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
    auto local_parser_rating_sfv = [] (const std::string & line) {
      std::vector<std::string> tmp = paracel::str_split(line, ',');
      std::vector<std::string> r({tmp[1], tmp[0], tmp[2]});
      return r;
    };
    auto rating_parser_func = paracel::gen_parser(local_parser_rating);
    if(fmt == "sfv") {
      rating_parser_func = paracel::gen_parser(local_parser_rating_sfv);
    }
    paracel_load_as_graph(rating_G,
                          rating_input,
                          rating_parser_func,
                          fmt);

    // init rating_G 
    paracel::dict_type<std::string, double> tmp_msg;
    auto init_lambda = [&] (const node_t & uid,
                            const node_t & iid,
                            double v) {
      std::string key = std::to_string(uid) + "_" + std::to_string(iid);
      tmp_msg[key] = v;
    };
    rating_G.traverse(init_lambda);
    paracel_write_multi(tmp_msg);
    paracel_sync();

    // learning
    cal_low_peak();
  }

  double cal_rmse() {
    auto worker_comm = get_comm();
    long long sz_sum = rating_G.e();
    worker_comm.allreduce(sz_sum);
    worker_comm.allreduce(training_rmse);
    return sqrt(training_rmse / sz_sum);
  }

  double cal_original_rmse() {
    auto worker_comm = get_comm();
    long long sz_sum = rating_G.e();
    worker_comm.allreduce(original_rmse);
    worker_comm.allreduce(sz_sum);
    return sqrt(original_rmse / sz_sum);
  }
  
  void dump_result() {
    paracel::dict_type<std::string, int> dump_result;
    for(auto & kv : ktop_result) {
      dump_result[std::to_string(kv.first)] = kv.second;
    }
    paracel_dump_dict(dump_result, "ktop_result_");
  }

 private:
  int linear_search(const node_t & node) {
    auto adj_table = sim_G.adjacent(node);
    paracel::list_type<std::pair<node_t, double> > ktop_list;
    for(auto & kv : adj_table) {
      ktop_list.push_back(std::make_pair(kv.first,
                                         kv.second));
    }
    auto comp_lambda = [] (std::pair<node_t, double> a,
                           std::pair<node_t, double> b) {
      return a.second > b.second;
    };
    std::sort(ktop_list.begin(), ktop_list.end(), comp_lambda);

    int ktop = -1;
    double res_min = DBL_MAX;
    paracel::dict_type<node_t, double> Ndict, Ddict;
    bool flag = false;
    
    for(size_t ktop_indx = low_limit - 1; ktop_indx < ktop_list.size(); ++ktop_indx) {

      double residual = 0.;
      node_t v = ktop_list[ktop_indx].first;
      double suv = ktop_list[ktop_indx].second;
      
      std::vector<std::string> keys;
      std::string key = std::to_string(v) + "_";
      for(auto & kv : rating_G.adjacent(node)) {
        keys.push_back(key + std::to_string(kv.first));
      }
      paracel::dict_type<std::string, double> vals;
      paracel_read_multi(keys, vals);
      
      for(auto & kv : rating_G.adjacent(node)) {
        node_t i = kv.first;
        double aui = kv.second;
        double avi = 0.;
        auto it = vals.find(key + std::to_string(i));
        if(it != vals.end()) {
          avi = it->second;
          Ndict[i] = Ndict[i] + avi * suv;
          Ddict[i] = Ddict[i] + suv;
        }
        if(Ndict.size() != 0 && Ddict[i] != 0) {
          residual += pow(aui - Ndict[i] / Ddict[i], 2.);
        } 
      } // for term
      if(ktop_indx == ktop_list.size() - 1) original_rmse += residual;

      if(flag == false || residual < res_min) {
        if(flag == false && residual == 0) continue;
        ktop = ktop_indx + 1;
        res_min = residual;
        flag = true;
      }
    }
    training_rmse += res_min;
    return ktop;
  }

  void cal_low_peak() {
    auto uid_set = sim_G.left_vertex_set();
    for(auto & uid : uid_set) {
      ktop_result[uid] = linear_search(uid);
      //std::cout << uid << ": " << ktop_result[uid] << std::endl;
    }
  }

 private:
  std::string rating_input, fmt;
  std::string sim_input;
  int low_limit = 1;
  paracel::bigraph<node_t> sim_G;
  paracel::bigraph<node_t> rating_G;
  paracel::dict_type<node_t, int> ktop_result;
  double training_rmse = 0., original_rmse = 0.;
}; // class adjust_ktop_s

} // namespace alg
} // namespace paracel

#endif
