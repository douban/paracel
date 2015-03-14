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

#include <vector>
#include <string>
#include <unordered_map>
#include "proxy.hpp"
#include "paracel_types.hpp"
#include "utils.hpp"

using std::vector;

extern "C" {
  extern paracel::update_result wc_updater_slow;
  extern paracel::update_result wc_updater;
  extern paracel::filter_with_key_result wc_filter;
}

int local_update_slow(int a, int b) {
  return a + b;
}

std::unordered_map<std::string, int>
local_update(const std::unordered_map<std::string, int> & a,
              const std::unordered_map<std::string, int> & b) {
  std::unordered_map<std::string, int> r(a);
  for(auto & kv : b) {
    auto key = kv.first;
    if(r.count(key)) {
      r[key] += kv.second;
    } else {
      r[key] = kv.second;
    }
  }
  return r;
}

bool local_filter(const std::string & key) {
  std::string s = "key_";
  if(paracel::startswith(key, s)) {
    return true;
  }
  return false;
}

paracel::update_result wc_updater_slow = paracel::update_proxy(local_update_slow);
paracel::update_result wc_updater = paracel::update_proxy(local_update);
paracel::filter_with_key_result wc_filter = paracel::filter_with_key_proxy(local_filter);
