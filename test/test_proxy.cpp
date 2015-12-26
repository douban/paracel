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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE PROXY_TEST 

#include <boost/test/unit_test.hpp>

#include <string>
#include <functional>
#include "proxy.hpp"
#include "paracel_types.hpp"
#include "test.hpp"

double local_incr(double v, double d) {
 return v + d;
}

using update_result = std::function<std::string(std::string, std::string)>;

double update(std::string key,
              std::string delta,
              update_result update_func){
  std::string val;
  double v = 3.21;
  paracel::packer<double> pk(v);
  pk.pack(val);
  std::string new_val = update_func(val, delta);
  paracel::packer<double> pk2;
  return pk2.unpack(new_val);
}

BOOST_AUTO_TEST_CASE (proxy_test) {
  update_result update_func = paracel::update_proxy(local_incr);
  std::string key("hello");
  double a = 1.23;
  paracel::packer<double> pk(a);
  std::string delta;
  pk.pack(delta);
  double r = update(key, delta, update_func);
  PARACEL_CHECK_EQUAL(r, 1.23 + 3.21);
}
