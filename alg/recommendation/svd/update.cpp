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
#include "proxy.hpp"
#include "paracel_types.hpp"

using std::vector;

extern "C" {
  extern paracel::update_result svd_updater;
}

vector<double> local_update(const vector<double> & a,
                            const vector<double> & b) {
  std::vector<double> r;
  for(size_t i = 0; i < a.size(); ++i) {
    r.push_back(a[i] + b[i]);
  }
  return r;
}


paracel::update_result svd_updater = paracel::update_proxy(local_update);
