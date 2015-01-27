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

// For fset format usage

#ifndef FILE_c48578a7_56e1_9287_8693_701d15c90268_HPP
#define FILE_c48578a7_56e1_9287_8693_701d15c90268_HPP

#include <cmath>
#include <queue>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <unordered_map>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"

using std::string;
using std::vector;
using std::unordered_map;

namespace paracel {
namespace alg {

using node_t = paracel::default_id_type;

struct min_heap_cmp {
  inline bool operator() (const std::pair<node_t, double> & l,
                          const std::pair<node_t, double> & r) {
    return l.second > r.second;
  }
};

struct heap_node {
  heap_node(node_t id, double v) {
    val = std::pair<node_t, double>(id, v);
  }
  std::pair<node_t, double> val;
};

using min_heap = std::priority_queue<std::pair<node_t, double>,
                                    vector<std::pair<node_t, double> >,
                                    min_heap_cmp>;

class sim_dense : public paracel::paralg {
 public:
  sim_dense(paracel::Comm comm,
            string hosts_dct_str,
            string _input,
            string _output,
            double _simbar = 0.0,
            int _ktop = 20)
      : paracel::paralg(hosts_dct_str, comm, _output),
        input(_input),
        output(_output),
        simbar(_simbar),
        ktop(_ktop) {}

  virtual ~sim_dense() {}

  double cal_sim(const vector<double> & a,
                 const vector<double> & b) {
    return paracel::dot_product(a, b);
  }

  void learning(const unordered_map<node_t, vector<double> > & var) {
    for(auto & iv : item_vects) {
      for(auto & jv : var) {
        if(iv.first != jv.first) {
          double sim = cal_sim(iv.second, jv.second);
          if(sim >= simbar) {
            auto hnode = heap_node(jv.first, sim);
            heapmap[iv.first].push(hnode.val);
            if((int)heapmap[iv.first].size() > ktop) heapmap[iv.first].pop();
            /*
            result[iv.first].push_back(
                std::make_pair(jv.first, sim));
            */
          }
        }
      }
    } // for iv
  }

  void select_top() {
    auto comp = [] (std::pair<node_t, double> a,
                    std::pair<node_t, double> b) {
      return std::get<1>(a) > std::get<1>(b);
    };
    for(auto & iv : item_vects) {
      if(!result.count(iv.first)) continue;
      std::sort(result[iv.first].begin(), result[iv.first].end(), comp);
      if(result[iv.first].size() > (size_t)ktop) {
        result[iv.first].resize(ktop);
      }
    }
  }

  void dump_result() {
    for(auto & kv : heapmap) {
      result[kv.first].resize(0);
      while(!heapmap[kv.first].empty()) {
        auto pr = heapmap[kv.first].top();
        result[kv.first].push_back(std::make_pair(pr.first, pr.second));
        heapmap[kv.first].pop();
      }
      std::reverse(result[kv.first].begin(), result[kv.first].end());
    }
    paracel_dump_dict(result);
  }

  void normalize(unordered_map<node_t, vector<double> > & v) {
    for(auto & kv : v) {
      double square_sum = 0.;
      // calc square sum
      for(size_t i = 0; i < kv.second.size(); ++i) {
        square_sum += std::pow(kv.second[i], 2);
      }
      double denominator = std::sqrt(square_sum);
      // divide
      for(auto it = kv.second.begin(); it != kv.second.end(); ++it) {
        *it = *it / denominator;
      }
    } // for
  }

  virtual void solve() {
    auto local_parser = [&] (const string & line) {
      vector<double> tmp;
      auto v = paracel::str_split(line, ',');
      for(size_t i = 1; i < v.size(); ++i) {
        tmp.push_back(std::stod(v[i]));
      }
      item_vects[paracel::cvt(v[0])] = tmp;
    };
    paracel_load_handle(input, local_parser);
    normalize(item_vects);

    auto handler = [&] (const vector<string> & linelst) {
      unordered_map<node_t, vector<double> > all_item_vects;
      for(auto & line : linelst) {
        vector<double> tmp;
        auto v = paracel::str_split(line, ',');
        for(size_t i = 1; i < v.size(); ++i) {
          tmp.push_back(std::stod(v[i]));
        }
        all_item_vects[paracel::cvt(v[0])] = tmp;
      }
      normalize(all_item_vects);
      learning(all_item_vects);
      //select_top();
    };
    paracel_sequential_loadall(input, handler);
  }
  
 private:
  string input;
  string output;
  double simbar = 0.; // lower bound similarity
  int ktop = 20;
  unordered_map<node_t, vector<double> > item_vects;
  unordered_map<node_t, vector<std::pair<node_t, double> > > result;
  unordered_map<node_t, min_heap> heapmap;
}; // class sim_dense

} // namespace alg
} // namespace paracel

#endif
