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
  extern paracel::update_result mf_fac_updater;
  extern paracel::update_result mf_bias_updater;
  extern paracel::update_result cnt_updater;
}

vector<double> local_update_fac(const vector<double> & a,
                                const vector<double> & b) {
  vector<double> r;
  for(size_t i = 0; i < a.size(); ++i) {
    r.push_back(a[i] + b[i]);
  }
  return r;
}

double local_update_bias(double a, double b) {
  return a + b;
}

int local_cnt_update(int a, int b) {
  return a + b;
}

paracel::update_result mf_fac_updater = paracel::update_proxy(local_update_fac);
paracel::update_result mf_bias_updater = paracel::update_proxy(local_update_bias);
paracel::update_result cnt_updater = paracel::update_proxy(local_cnt_update);
