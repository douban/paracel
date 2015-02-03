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
#include "utils.hpp"
#include "proxy.hpp"
#include "paracel_types.hpp"

using std::vector;
using std::pair;
using std::string;
using node_t = paracel::default_id_type;

extern "C" {
  extern paracel::update_result init_updater;
  extern paracel::filter_with_key_result pr_filter;
}

vector<pair<node_t, double> >
local_update(const vector<pair<node_t, double> > & a,
             const vector<pair<node_t, double> > & b) {
  vector<pair<node_t, double> > r(a);
  r.insert(r.end(), b.begin(), b.end());
  return r;
}

bool local_filter(const std::string & key) {
  string s = "_pr";
  if(paracel::endswith(key, s)) {
    return true;
  }
  return false;
}

paracel::update_result init_updater = paracel::update_proxy(local_update);
paracel::filter_with_key_result pr_filter = paracel::filter_with_key_proxy(local_filter);
