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

#include "proxy.hpp"
#include "paracel_types.hpp"

extern "C" {
  extern paracel::update_result max_updater;
}

double local_update(double a, double b) {
  return a > b ? a : b;
}

paracel::update_result max_updater = paracel::update_proxy(local_update);
