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

#ifndef FILE_a803dadb_1745_df69_9448_e3c1619d141e_HPP
#define FILE_a803dadb_1745_df69_9448_e3c1619d141e_HPP

#define sentinel 0xFFFFFFFFFFFFFFFF

#include <cmath>
#include <cfloat>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include "ps.hpp"
#include "load.hpp"

namespace paracel {
namespace alg {

using node_t = paracel::default_id_type;

class decision_tree {
  public:
   decision_tree() {
     T.resize(0);
   }
   ~decision_tree() {}
   void add(node_t id) {
     T.push_back(id);
   } 
   std::vector<node_t> get_tree() {
     return T;
   }
  private:
   std::vector<node_t> T;
};

class decision_tree_builder_factor : public paracel::paralg {
 
 public:
  decision_tree_builder_factor(paracel::Comm comm,
                               std::string hosts_dct_str,
                               std::string _uinput,
                               std::string _iinput,
                               std::string _output,
                               std::string handle_fn,
                               int _level,
                               int _tree_num)
      : paracel::paralg(hosts_dct_str, comm, _output),
        uinput(_uinput),
        iinput(_iinput),
        output(_output),
        handle_file(handle_fn),
        level(_level),
        N(_tree_num) {
    trees.resize(N);
    avg_ufacs.resize(N);
  }

  virtual ~decision_tree_builder_factor() {}

  void solve() {
    init_ufac(uinput);
    init_ifac(iinput);
    std::unordered_set<node_t> users;
    for(size_t i = 0; i < ufac.size(); ++i) {
      users.insert(i);
    }
    if(get_worker_id() == 0) {
      for(int n = 0; n < N; ++n) {
        avg_ufacs[n].push_back(cal_avg_pu(users));
      }
      std::cout << "init done" << std::endl;
    }
    paracel_sync();
    for(int k = 0; k < N; ++k) {
      build(k, users);
      paracel_sync();
    }
  }

  void dump() {
    if(get_worker_id() == 0) {
      std::ofstream os;
      os.open(paracel::todir(output) + "tree_0");
      for(int k = 0; k < N; ++k) {
        auto tree_data = trees[k].get_tree();
        os << "tree_" << std::to_string(k) << '\t';
        for(size_t i = 0; i < tree_data.size() - 1; ++i) {
          os << std::to_string(tree_data[i]) << "|";
        }
        os << std::to_string(tree_data.back()) << '\n';
      }
      os.close();
      for(int n = 0; n < N; ++n) {
        dump_avg_user_fac(n);
      }
    }
  }

  void dump_avg_user_fac(int k) {
    std::ofstream os;
    os.open(paracel::todir(output) + "average_user_factor_" + std::to_string(k));
    for(int i = 0; i < ((1 << level) - 1); ++i) {
      for(int j = 0; j < kdim - 1; ++j) {
        os << std::to_string(avg_ufacs[k][i][j]) << "|";
      }
      os << std::to_string(avg_ufacs[k][i].back()) << '\n';
    }
    os.close();
  }

 private:
  // id fac1|fac2|fac3...
  void init_ufac(const std::string & fn) {
    ufac.resize(0);
    auto lines = paracel_loadall(fn);
    for(auto & line : lines) {
      auto tmp = paracel::str_split(line, '\t');
      auto facs = paracel::str_split(tmp[1], '|');
      if(kdim == -1) kdim = facs.size();
      std::vector<double> facs_tmp;
      for(auto & fac : facs) {
        facs_tmp.push_back(std::stod(fac));
        //ufac[paracel::cvt(tmp[0])].push_back(std::stod(fac));
      }
      ufac.push_back(facs_tmp);
    }
  }
  
  // id fac1|fac2|fac3...
  void init_ifac(const std::string & fn) {
    ifac.clear();
    std::unordered_map<std::string, std::vector<double> > ifac_tmp;
    auto lines = paracel_load(fn);
    for(auto & line : lines) {
      auto tmp = paracel::str_split(line, '\t');
      auto facs = paracel::str_split(tmp[1], '|');
      for(auto & fac : facs) {
        ifac[paracel::cvt(tmp[0])].push_back(std::stod(fac));
        ifac_tmp["ifac_" + tmp[0]].push_back(std::stod(fac));
      }
    }
    paracel_write_multi(ifac_tmp);
  }

  double vec_err(const std::vector<double> & a, const std::vector<double> & b) {
    double r = 0.;
    for(int i = 0; i < kdim; ++i) {
      r += pow(a[i] - b[i], 2.);
    }
    return r;
  }

  void build(int tree_indx,
             const std::unordered_set<node_t> & U) {
    std::queue<std::unordered_set<node_t> > Q;
    Q.push(U);
    int cnt = 0;
    std::unordered_set<node_t> occur, Ulike, Uhate;
    while(!Q.empty()) {
      auto UU = Q.front(); Q.pop();
      auto partition_id = find_node(tree_indx, UU, occur, Q);
      occur.insert(partition_id);
      trees[tree_indx].add(partition_id);
      cnt += 1;
      if(get_worker_id() == 0) std::cout << cnt << " build done" << partition_id << std::endl;
      if(cnt >= ((1 << level) - 1)) break;
    }
  }

  node_t find_node(int tree_indx,
                   const std::unordered_set<node_t> & U,
                   const std::unordered_set<node_t> & occur,
                   std::queue<std::unordered_set<node_t> > & Q) {
    if(U.size() == 0) {
      std::unordered_set<node_t> U_like_r, U_hate_r;
      Q.push(U_like_r);
      Q.push(U_hate_r);
      return sentinel;
    }
    std::unordered_map<node_t, double> global_err;
    for(auto & kv : ifac) {
      node_t iid = kv.first;
      if(occur.count(iid)) continue;
      std::unordered_set<node_t> U_like, U_hate;
      for(auto & uid : U) {
        if(paracel::dot_product(kv.second, ufac[uid]) >= 0.) {
          U_like.insert(uid);
        } else {
          U_hate.insert(uid);
        }
      }

      if(U_like.size() == 0 || U_hate.size() == 0) continue;
      if(U_like.size() != 0 && U_hate.size() != 0) {
        if((U_like.size() / U_hate.size() >= 3) 
           || (U_hate.size() / U_like.size() >= 3)) {
          continue;
        }
      }

      auto avg_pu_like = cal_avg_pu(U_like);
      auto avg_pu_hate = cal_avg_pu(U_hate);
      double err_like = 0., err_hate = 0.;
      for(auto & u : U_like) {
        err_like += vec_err(ufac[u], avg_pu_like);
      }
      for(auto & u : U_hate) {
        err_hate += vec_err(ufac[u], avg_pu_hate);
      }
      global_err[iid] = err_like + err_hate;
      //std::cout << iid << "|" << global_err[iid] << "\t" << err_like << "|" << U_like.size() << "\t" << err_hate << "|" << U_hate.size() << std::endl;
    } // item

    node_t partition_id = sentinel;
    if(Q.size() == 0) {
      std::vector<std::pair<node_t, double> > tmp;
      for(auto & kv : global_err) {
        tmp.push_back(std::make_pair(kv.first, kv.second));
      }
      auto cmp_lambda = [] (std::pair<node_t, double> a, std::pair<node_t, double> b) {
        return a.second < b.second;
      };
      std::sort(tmp.begin(), tmp.end(), cmp_lambda);
      std::unordered_map<std::string, double> err_tmp;
      for(int ii = 0; ii <= tree_indx; ++ii) {
        err_tmp["err_" + std::to_string(tmp[ii].first)] = tmp[ii].second;
      }
      paracel_write_multi(err_tmp);
      paracel_sync();
      std::string filter_func = "small_errors_filter";
      auto smallest_errs = paracel_read_special<double>(handle_file, filter_func);
      std::vector<std::pair<std::string, double> > tmp_container;
      for(auto & kv : smallest_errs) {
        tmp_container.push_back(std::make_pair(kv.first, kv.second));
      }
      auto cmp_lambda2 = [] (std::pair<std::string, double> a,
                             std::pair<std::string, double> b) {
        return a.second < b.second;
      };
      std::sort(tmp_container.begin(), tmp_container.end(), cmp_lambda2);
      if(global_err.size() != 0) {
        std::string key = tmp_container[tree_indx].first;
        partition_id = paracel::cvt(key.substr(4, key.size() - 4));
      }
      paracel_sync();
      for(auto & kv : err_tmp) {
        paracel_remove(kv.first);
      }
    } else {
      double min_err = DBL_MAX;
      for(auto & kv : global_err) {
        if(kv.second < min_err) {
          partition_id = kv.first;
          min_err = kv.second;
        }
      }
      //paracel_write("smallest_error", std::make_pair(sentinel, DBL_MAX));
      paracel_sync();
      paracel_bupdate("smallest_error",
                      std::make_pair(partition_id, min_err),
                      handle_file,
                      "smallest_error_updater");
      paracel_sync();
      auto smallest = paracel_read<std::pair<node_t, double> >("smallest_error");
      paracel_sync();
      paracel_remove("smallest_error");
      partition_id = smallest.first;
    }

    std::unordered_set<node_t> U_like_r, U_hate_r;
    if(global_err.size() == 0) {
      Q.push(U_like_r);
      Q.push(U_hate_r);
      return partition_id;
    }
    std::vector<double> p_i(kdim, 0.);
    auto it = ifac.find(partition_id);
    if(it != ifac.end()) {
      p_i = ifac[partition_id];
    } else {
      p_i = paracel_read<std::vector<double> >("ifac_" + std::to_string(partition_id));
    }
    for(auto & uid : U) {
      if(paracel::dot_product(p_i, ufac[uid]) >= 0.) {
        U_like_r.insert(uid);
      } else {
        U_hate_r.insert(uid);
      }
    }
    if(get_worker_id() == 0) {
      avg_ufacs[tree_indx].push_back(cal_avg_pu(U_like_r));
      avg_ufacs[tree_indx].push_back(cal_avg_pu(U_hate_r));
    }
    Q.push(U_like_r);
    Q.push(U_hate_r);
    return partition_id;
  }

  std::vector<double> cal_avg_pu(const std::unordered_set<node_t> & uset) {
    std::vector<double> result(kdim, 0.);
    for(auto & u : uset) {
      auto p_u = ufac[u];
      for(int k = 0; k < kdim; ++k) {
        result[k] += p_u[k];
      }
    }
    if(uset.size() != 0) {
      for(int k = 0; k < kdim; ++k) {
        result[k] /= uset.size();
      }
    }
    return result;
  }

 private:
  std::string uinput, iinput;
  std::string output;
  std::string handle_file;
  int level;
  int N;
  int kdim = -1;
  std::unordered_map<node_t, std::vector<double> > ifac;
  std::vector<std::vector<double> > ufac;
  std::vector<decision_tree> trees;
  std::vector<std::vector<std::vector<double> > > avg_ufacs;
}; // class decision_tree_builder_factor

} // namespace alg
} // namespace paracel

#endif
