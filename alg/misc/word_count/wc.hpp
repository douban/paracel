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

#ifndef FILE_da1cd0b3_ecfe_3fcd_43e9_aa5c48ace349_HPP
#define FILE_da1cd0b3_ecfe_3fcd_43e9_aa5c48ace349_HPP

#include <queue>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"
#include "paracel_types.hpp"

namespace paracel {
namespace alg {

struct local_min_heap_cmp {
  inline bool operator() (const std::pair<std::string, int> & l,
                          const std::pair<std::string, int> & r) {
    return l.second > r.second;
  }
};

class word_count : public paracel::paralg {
  
 public:
  word_count(paracel::Comm comm,
             std::string hosts_dct_str,
             std::string _input,
             std::string _output,
             int k,
             std::string handle_fn,
             std::string update_fcn,
             std::string filter_fcn) :
      paracel::paralg(hosts_dct_str, comm, _output),
      input(_input),
      topk(k),
      handle_file(handle_fn),
      update_function(update_fcn),
      filter_function(filter_fcn) {}

  virtual ~word_count() {}

  void learning_slow_demo(const std::vector<std::string> & lines) {
    for(auto & line : lines) {
      auto word_lst = parser(line);
      for(auto & word : word_lst) {
        paracel_bupdate(word, 1, handle_file, "wc_updater_slow");
      }
    }
    paracel_sync();
    paracel_read_topk(topk, result);
  }
  
  void learning_slow(const std::vector<std::string> & lines) {
    std::unordered_map<std::string, int> tmp;
    for(auto & line : lines) {
      auto word_lst = parser(line);
      for(auto & word : word_lst) {
        tmp[word] += 1;
      }
    }
    for(auto & wc : tmp) {
      paracel_bupdate(wc.first,
                      wc.second,
                      handle_file,
                      "wc_updater_slow");
    }
    paracel_sync();
    paracel_read_topk(topk, result);
  }

  void learning(const std::vector<std::string> & lines) {
    std::unordered_map<std::string, int> tmp;
    for(auto & line : lines) {
      auto word_lst = parser(line);
      for(auto & word : word_lst) {
        tmp[word] += 1;
      }
    }
    int dct_sz = 100;
    std::vector<std::unordered_map<std::string, int> > local_vd(dct_sz);
    paracel::hash_type<std::string> hfunc;
    for(auto & kv : tmp) {
      auto indx = hfunc(kv.first) % dct_sz;
      local_vd[indx][kv.first] = kv.second;
    }
    paracel_sync();
    for(int k = 0; k < dct_sz; ++k) {
      paracel_bupdate("key_" + std::to_string(k),
                      local_vd[k],
                      handle_file,
                      update_function);
    }
    paracel_sync();

    // get topk
    using min_heap = std::priority_queue<
                                        std::pair<std::string, int>,
                                        std::vector<std::pair<std::string, int> >,
                                        local_min_heap_cmp>;
            
    min_heap tmplst;
    auto handler = [&] (const std::unordered_map<std::string,
                        std::unordered_map<std::string, int> > & d) {
      for(auto & kv : d) {
        for(auto & kkv : kv.second) {
          auto node = paracel::heap_node<paracel::str_type, int>(kkv.first,
                                                                 kkv.second);
          tmplst.push(node.val);
          if((int)tmplst.size() > topk) {
            tmplst.pop();
          }
        }
      }
    };  
    paracel_read_special_handle<std::unordered_map<std::string, int> >(handle_file,
                                                                       filter_function,
                                                                       handler);
    result.resize(0);
    while(!tmplst.empty()) {
      result.push_back(std::make_pair(tmplst.top().first,
                                      tmplst.top().second));
      tmplst.pop();
    }   
    std::reverse(result.begin(), result.end());
  }

  virtual void solve() {
    auto lines = paracel_load(input);
    paracel_sync();
    learning(lines);
    paracel_sync();
  }

  void print() {
    if(get_worker_id() == 0) {
      for(auto & pr : result) {
        std::cout << std::get<0>(pr) << " : " 
            << std::get<1>(pr) << std::endl;
      }
    }
  }

 private:
  std::vector<std::string> parser(const std::string & line) {
    std::vector<std::string> wl, rl;
    boost::algorithm::split_regex(wl, line, boost::regex("[^-a-zA-Z0-9_]"));
    for(size_t i = 0; i < wl.size(); ++i) {
      if(wl[i] != "") {
        rl.push_back(wl[i]);
      }
    }
    return rl;
  }

 private:
  std::string input;
  int topk = 10;
  std::string handle_file;
  std::string update_function;
  std::string filter_function;
  std::vector<std::pair<std::string, int> > result;
}; // class word_count

} // namespace alg
} // namespace paracel

#endif
