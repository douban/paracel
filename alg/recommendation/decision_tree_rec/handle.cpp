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
#include "proxy.hpp"
#include "paracel_types.hpp"
#include "utils.hpp"

extern "C" {
  extern paracel::update_result smallest_error_updater;
  extern paracel::filter_with_key_result small_errors_filter;
}

std::pair<paracel::default_id_type, double>
local_update(std::pair<paracel::default_id_type, double> a,
             std::pair<paracel::default_id_type, double> b) {
  if(a.second <= b.second) {
    return a;
  }
  return b;
}

bool local_filter(const std::string & key) {
  std::string s = "err_";
  if(paracel::startswith(key, s)) {
    return true;
  }
  return false;
}

paracel::update_result smallest_error_updater = paracel::update_proxy(local_update);
paracel::filter_with_key_result small_errors_filter = paracel::filter_with_key_proxy(local_filter);
