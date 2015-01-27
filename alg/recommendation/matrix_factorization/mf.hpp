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

#ifndef FILE_ce4b7e1a_e522_9f78_ee51_c53f717fb82e_HPP
#define FILE_ce4b7e1a_e522_9f78_ee51_c53f717fb82e_HPP

#include <cmath>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

#include "ps.hpp"
#include "utils.hpp"
#include "graph.hpp"

using std::string;
using std::vector;
using std::unordered_map;

namespace paracel {
namespace alg {

using node_t = paracel::default_id_type;

// return vector<string> in your parser 
auto local_parser = [] (const std::string & line) {
  auto lst = paracel::str_split(line, ' ');
  lst.erase(lst.begin() + 2);
  return lst;
};

class matrix_factorization : public paracel::paralg {

 public:
  matrix_factorization(paracel::Comm comm,
                       string hosts_dct_str,
                       string _input,
                       string _predict_input,
                       string _output,
                       string update_fn,
                       vector<string> update_fcn_lst,
                       string filter_fn,
                       vector<string> filter_fcn_lst,
                       int k = 80,
                       int _rounds = 1,
                       double _alpha = 0.005,
                       double _beta = 0.01,
                       bool _debug = false,
                       bool ssp_switch = true,
                       int limit_s = 3) 
      : paracel::paralg(hosts_dct_str,
                        comm,
                        _output,
                        _rounds,
                        limit_s,
                        ssp_switch),
      input(_input),
      predict_input(_predict_input),
      update_file(update_fn),
      update_funcs(update_fcn_lst),
      filter_file(filter_fn),
      filter_funcs(filter_fcn_lst),
      fac_dim(k),
      rounds(_rounds),
      alpha(_alpha),
      beta(_beta),
      debug(_debug) { loss_error_train.resize(0); }

  virtual ~matrix_factorization() {}

  inline double estimate(const node_t & uid,
                         const node_t & iid) {
    double predict = miu
        + usr_bias[uid] + item_bias[iid]
        + paracel::dot_product(W[uid], H[iid]);
    predict = std::min(5., predict);
    predict = std::max(1., predict);
    return predict;
  }

  double cal_rmse(paracel::bigraph<node_t> & graph) {
    auto worker_comm = get_comm();
    double rmse = 0.;
    auto rmse_lambda = [&] (const node_t & uid,
                            const node_t & iid,
                            double rating) {
      double e = rating - estimate(uid, iid);
      rmse += e * e;
    };
    graph.traverse(rmse_lambda);
    worker_comm.allreduce(rmse);
    long long sz_sum = graph.e();
    worker_comm.allreduce(sz_sum);
    return sqrt(rmse / sz_sum);
  }

  double cal_rmse() {
    return cal_rmse(rating_graph);
  }

  void init_parameters() {
    auto worker_comm = get_comm(); 
    auto f_parser = paracel::gen_parser(local_parser);
    paracel_load_as_graph(rating_graph,
                          input,
                          f_parser,
                          "fsmap");
    rating_sz = rating_graph.e();
    auto init_lambda = [&] (const node_t & a,
                            const node_t & b,
                            double c) {
      usr_bag.insert(a);
      item_bag.insert(b);
      miu += c;
    };
    rating_graph.traverse(init_lambda);
    sync();
    worker_comm.allreduce(miu);
    long long rating_sz_tmp = rating_sz;
    worker_comm.allreduce(rating_sz_tmp);
    miu /= static_cast<double>(rating_sz_tmp);
    
    for(auto & usr : usr_bag) {
      W[usr] = paracel::random_double_list(fac_dim, 0.1);
      usr_bias[usr] = 0.1 * paracel::random_double();
    }
    for(auto & item : item_bag) {
      H[item] = paracel::random_double_list(fac_dim, 0.1);
      item_bias[item] = 0.1 * paracel::random_double();
    }

    // first push
    id = get_worker_id();
    paracel::dict_type<paracel::str_type,
                      paracel::list_type<double> > local_W_dct, local_H_dct;
    paracel::dict_type<paracel::str_type, double> local_ub_dct, local_ib_dct;
    for(auto & uid : usr_bag) {
      std::string W_key = "W_" + cvt(uid);
      std::string ub_key = "usr_bias_" + cvt(uid);
      local_W_dct[W_key] = W[uid];
      local_ub_dct[ub_key] = usr_bias[uid];
      paracel_bupdate(cvt(uid) + "_u_cnt",
                      1,
                      update_file,
                      update_funcs[0]); // cnt_updater
    }
    paracel_write_multi(local_W_dct);
    paracel_write_multi(local_ub_dct);
    for(auto & iid : item_bag) {
      std::string H_key = "H_" + cvt(iid);
      std::string ib_key = "item_bias_" + cvt(iid);
      local_H_dct[H_key] = H[iid];
      local_ib_dct[ib_key] = item_bias[iid];
      paracel_bupdate(cvt(iid) + "_i_cnt",
                      1,
                      update_file,
                      update_funcs[0]); // cnt_updater
    }
    paracel_write_multi(local_H_dct);
    paracel_write_multi(local_ib_dct);
    sync();
    if(id == 0) std::cout << "first push done" << std::endl;
    
    paracel::list_type<paracel::str_type> tmp_wgtx_lst, tmp_wgty_lst;
    paracel::list_type<int> tmp_x_cnt, tmp_y_cnt;
    for(auto & u : usr_bag) {
      tmp_wgtx_lst.push_back(cvt(u) + "_u_cnt");
    }
    for(auto & i : item_bag) {
      tmp_wgty_lst.push_back(cvt(i) + "_i_cnt");
    }
    tmp_x_cnt = paracel_read_multi<int>(tmp_wgtx_lst);
    tmp_y_cnt = paracel_read_multi<int>(tmp_wgty_lst);

    size_t indx_cnt = 0;
    for(auto & u : usr_bag) {
      wgtx_map[u] = 1. / static_cast<double>(tmp_x_cnt[indx_cnt]);
      indx_cnt += 1;
    }
    indx_cnt = 0;
    for(auto & i : item_bag) {
      wgty_map[i] = 1. / static_cast<double>(tmp_y_cnt[indx_cnt]);
      indx_cnt += 1;
    }
    sync();
    if(id == 0) std::cout << "initialization done" << std::endl;
  }

  void read_mf_paras() {
    for(auto & uid : usr_bag) {
      std::string W_key = "W_" + cvt(uid);
      std::string ub_key = "usr_bias_" + cvt(uid);
      W[uid] = paracel_read<vector<double> >(W_key);
      usr_bias[uid] = paracel_read<double>(ub_key);
    }
    for(auto & iid : item_bag) {
      std::string H_key = "H_" + cvt(iid);
      std::string ib_key = "item_bias_" + cvt(iid);
      H[iid] = paracel_read<vector<double> >(H_key);
      item_bias[iid] = paracel_read<double>(ib_key);
    }
  }

  void update_mf_fac(unordered_map<node_t, vector<double> > & old_W,
                     unordered_map<node_t, vector<double> > & old_H) {
    paracel::str_type file_name = update_file;
    paracel::str_type func_name = update_funcs[1]; // mf_fac_updater
    vector<double> fac_delta(fac_dim);
    for(auto & uid : usr_bag) {
      std::string W_key = "W_" + cvt(uid);
      for(int i = 0; i < fac_dim; ++i) {
        fac_delta[i] = wgtx_map[uid] * (W[uid][i] - old_W[uid][i]);
      }
      paracel_bupdate(W_key,
                      fac_delta,
                      file_name,
                      func_name);
    }
    for(auto & iid : item_bag) {
      std::string H_key = "H_" + cvt(iid);
      for(int i = 0; i < fac_dim; ++i) {
        fac_delta[i] = wgty_map[iid] * (H[iid][i] - old_H[iid][i]);
      }
      paracel_bupdate(H_key,
                      fac_delta,
                      file_name,
                      func_name);
    }
  }

  void update_mf_bias(unordered_map<node_t, double> & old_ubias, 
                      unordered_map<node_t, double> & old_ibias) {
    paracel::str_type file_name = update_file;
    paracel::str_type func_name = update_funcs[2]; // mf_bias_updater
    for(auto & uid : usr_bag) {
      std::string ub_key = "usr_bias_" + cvt(uid);
      paracel_bupdate(ub_key,
                      wgtx_map[uid] * (usr_bias[uid] - old_ubias[uid]),
                      file_name,
                      func_name);
    }
    for(auto & iid : item_bag) {
      std::string ib_key = "item_bias_" + cvt(iid);
      paracel_bupdate(ib_key,
                      wgty_map[iid] * (item_bias[iid] - old_ibias[iid]),
                      file_name,
                      func_name);
    }
  }

  void learning() {
    std::vector<double> delta_W(fac_dim), delta_H(fac_dim);
    auto kernel_lambda = [&] (const node_t & uid,
                              const node_t & iid,
                              double rating) {
      double e = rating - estimate(uid, iid);
      for(int i = 0; i < fac_dim; ++i) {
        delta_W[i] = alpha * (2 * e * H[iid][i] - beta * W[uid][i]);
        delta_H[i] = alpha * (2 * e * W[uid][i] - beta * H[iid][i]);
      }
      for(int i = 0; i < fac_dim; ++i) {
        W[uid][i] += delta_W[i];
        H[iid][i] += delta_H[i];
      }
      usr_bias[uid] += alpha * (2 * e - beta * usr_bias[uid]);
      item_bias[iid] += alpha * (2 * e - beta * item_bias[iid]);
    };
    unordered_map<node_t, vector<double> > old_W, old_H;
    unordered_map<node_t, double> old_ubias, old_ibias;

    // main loop
    for(int rd = 0; rd < rounds; ++rd) {
      // read paras from servers
      read_mf_paras();
      if(debug) { 
        loss_error_train.push_back(cal_rmse(rating_graph));
      }
      
      // record locally
      for(auto & uid : usr_bag) {
        if(old_W[uid].size() != (size_t)fac_dim) {
          old_W[uid].resize(fac_dim);
        }
        for(int i = 0; i < fac_dim; ++i) {
          old_W[uid][i] = W[uid][i];
        }
        old_ubias[uid] = usr_bias[uid];
      }
      for(auto & iid : item_bag) {
        if(old_H[iid].size() != (size_t)fac_dim) {
          old_H[iid].resize(fac_dim);
        }
        for(int i = 0; i < fac_dim; ++i) {
          old_H[iid][i] = H[iid][i];
        }
        old_ibias[iid] = item_bias[iid];
      }
      
      // update paras locally
      rating_graph.traverse(kernel_lambda);
      
      // update paras to servers
      update_mf_fac(old_W, old_H);
      sync(); // important 
      
      update_mf_bias(old_ubias, old_ibias);
      sync();

      iter_commit();
      if(get_worker_id() == 0) std::cout << "finish round: " << rd << std::endl;
    }

    // last read
    read_mf_paras();
    usr_bag.clear();
    item_bag.clear();
    if(debug) { 
      loss_error_train.push_back(cal_rmse(rating_graph));
    }
    sync();
    if(get_worker_id() == 0) std::cout << "last pull finished" << std::endl;
  }

  void predict() {
    if(predict_input.size() == 0) return;
    auto lines = paracel_load(predict_input);
    auto tmp_W = paracel_read_special<vector<double> >(filter_file, filter_funcs[2]);
    auto tmp_H = paracel_read_special<vector<double> >(filter_file, filter_funcs[3]);
    auto tear_lambda = [] (const string & str) {
      auto pos1 = str.rfind('_') + 1;
      return str.substr(pos1, str.size());
    };
    for(auto & kv : tmp_W) {
      string temp = tear_lambda(kv.first);
      node_t uid;
      local_fmt(temp, uid);
      W[uid] = kv.second;
    }
    for(auto & kv : tmp_H) {
      string temp = tear_lambda(kv.first);
      node_t iid;
      local_fmt(temp, iid);
      H[iid] = kv.second;
    }

    for(auto & line : lines) {
      auto lst = paracel::str_split(line, ',');
      node_t uid, iid;
      local_fmt(lst[0], uid);
      local_fmt(lst[1], iid);
      if(!W.count(uid) || !H.count(iid)) {
        predv.push_back(std::make_tuple(uid, iid, miu));
      } else {
        predv.push_back(std::make_tuple(uid, iid, estimate(uid, iid)));
      }
    }
  }

  virtual void solve() {
    init_parameters(); 
    set_total_iters(rounds);
    sync();
    learning();
    sync();
  }

  void dump_result() {
    auto worker_comm = get_comm(); 
    long long rating_sz_tmp = rating_sz;
    worker_comm.allreduce(rating_sz_tmp);

    if(id == 0) {
      unordered_map<string, double> dump_miu;
      dump_miu["miu"] = miu;
      dump_miu["rating_sz"] = static_cast<double>(rating_sz_tmp);
      paracel_dump_dict(dump_miu, "miu_");
     
      auto tear_lambda = [] (const string & str) {
        auto pos1 = str.rfind('_') + 1;
        return str.substr(pos1, str.size());
      };
      auto lambda_ubias = [&] (unordered_map<string, double> & d) {
        unordered_map<string, double> dump_usr_bias;
        for(auto & kv : d) {
          dump_usr_bias[tear_lambda(kv.first)] = kv.second;
        }
        paracel_dump_dict(dump_usr_bias, "ubias_", true);
      };
      paracel_read_special_handle<double>(filter_file,
                                          filter_funcs[0],
                                          lambda_ubias); // mf_ubias_filter
      
      auto lambda_ibias = [&] (unordered_map<string, double> & d) {
        unordered_map<string, double> dump_item_bias;
        for(auto & kv : d) {
          dump_item_bias[tear_lambda(kv.first)] = kv.second;
        }
        paracel_dump_dict(dump_item_bias, "ibias_", true);
      };
      paracel_read_special_handle<double>(filter_file,
                                          filter_funcs[1],
                                          lambda_ibias); // mf_ibias_filter

      auto lambda_W = [&] (unordered_map<string, vector<double> > & d) {
        unordered_map<string, vector<double> > dump_W;
        for(auto & kv : d) {
          dump_W[tear_lambda(kv.first)] = kv.second;
        }
        paracel_dump_dict(dump_W, "W_", true);
      };
      paracel_read_special_handle<vector<double> >(filter_file,
                                                   filter_funcs[2],
                                                   lambda_W); // mf_W_filter

      auto lambda_H = [&] (unordered_map<string, vector<double> > & d) {
        unordered_map<string, vector<double> > dump_H;
        for(auto & kv : d) {
          dump_H[tear_lambda(kv.first)] = kv.second;
        }
        paracel_dump_dict(dump_H, "H_", true);
      };
      paracel_read_special_handle<vector<double> >(filter_file,
                                                   filter_funcs[3],
                                                   lambda_H); // mf_H_filter
      /*
      auto tmp_usr_bias = paracel_read_special<double>(filter_file,
                                                       filter_funcs[0]); // mf_ubias_filter
      auto tmp_item_bias = paracel_read_special<double>(filter_file,
                                                        filter_funcs[1]); // mf_ibias_filter
      auto tmp_W = paracel_read_special<vector<double> >(filter_file,
                                                         filter_funcs[2]); // mf_W_filter
      auto tmp_H = paracel_read_special<vector<double> >(filter_file,
                                                         filter_funcs[3]); // mf_H_filter
      
      unordered_map<string, double> dump_usr_bias, dump_item_bias;
      unordered_map<string, vector<double> > dump_W, dump_H;
      
      auto tear_lambda = [] (const string & str) {
        auto pos1 = str.rfind('_') + 1;
        return str.substr(pos1, str.size());
      };

      for(auto & kv : tmp_usr_bias) {
        string temp = tear_lambda(kv.first);
        dump_usr_bias[temp] = kv.second;
      }
      paracel_dump_dict(dump_usr_bias, "ubias_");
      dump_usr_bias.clear();

      for(auto & kv : tmp_item_bias) {
        string temp = tear_lambda(kv.first);
        dump_item_bias[temp] = kv.second;
      }
      paracel_dump_dict(dump_item_bias, "ibias_");
      dump_item_bias.clear();
      
      for(auto & kv : tmp_W) {
        string temp = tear_lambda(kv.first);
        dump_W[temp] = kv.second;
      }
      paracel_dump_dict(dump_W, "W_");
      dump_W.clear();

      for(auto & kv : tmp_H) {
        string temp = tear_lambda(kv.first);
        dump_H[temp] = kv.second;
      }
      paracel_dump_dict(dump_H, "H_");
      dump_H.clear();
      */

      if(debug) {
        paracel_dump_vector(loss_error_train, "mf_loss_error_train_", "\n");
      }
    }
    paracel_dump_vector(predv, "pred_v_", "\n");
    sync();
    if(id == 0) std::cout << "dump finished" << std::endl;
  }

 private:
  inline string cvt(paracel::default_id_type id) { return std::to_string(id); }
  inline string cvt(string id) { return id; }
  inline void local_fmt(string id, string & r) { r = id; }
  inline void local_fmt(string id, paracel::default_id_type & r) { r = paracel::cvt(id); }

 private:
  int id;
  string input, predict_input;
  string update_file;
  vector<string> update_funcs;
  string filter_file;
  vector<string> filter_funcs;
  int fac_dim; // factor dim
  int rounds;
  double alpha, beta;
  bool debug;
  
  vector<double> loss_error_train;
  long long rating_sz = 0;
  double miu = 0.;
  paracel::bigraph<node_t> rating_graph;
  
  unordered_map<node_t, double> wgtx_map, wgty_map;
  std::unordered_set<node_t> usr_bag, item_bag;
  unordered_map<node_t, vector<double> > W, H;
  unordered_map<node_t, double> usr_bias, item_bias;

  vector<std::tuple<node_t, node_t, double> > predv;
};

} // namespace alg
} // namespace paracel

#endif
