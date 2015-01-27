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
  extern paracel::update_result lr_theta_update;
}

vector<double> local_update(vector<double> a, vector<double> b) {
  vector<double> r;
  for(int i = 0; i < (int)a.size(); ++i) {
    r.push_back(a[i] + b[i]);
  }
  return r;
}

paracel::update_result lr_theta_update = paracel::update_proxy(local_update);
