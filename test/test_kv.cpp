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
#define BOOST_TEST_MODULE KV_TEST 

#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>
#include "test.hpp"
#include "kv.hpp"
#include "paracel_types.hpp"

BOOST_AUTO_TEST_CASE (kv_test) {
  paracel::kvs<char, int> obj;
  obj.set('a', 2);
  obj.set('b', 0);
  paracel::dict_type<char, int> tmp;
  tmp['x'] = 100; tmp['y'] = 200;
  obj.set_multi(tmp);
  if(auto v = obj.get('a')) {
    BOOST_CHECK_EQUAL(*v, 2);
  }
  if(auto v = obj.get('b')) {
    BOOST_CHECK_EQUAL(*v, 0);
  }
  int tval;
  obj.get('a', tval);
  BOOST_CHECK_EQUAL(tval, 2);
  if(auto v = obj.get('c')) {
    BOOST_CHECK_EQUAL(*v, 7);
  }

  auto dct = obj.getall();
  paracel::dict_type<char, int> rtmp;
  rtmp['a'] = 2; rtmp['b'] = 0; rtmp['x'] = 100; rtmp['y'] = 200;
  BOOST_CHECK_EQUAL_D(rtmp, dct);

  bool r = false;
  if(auto vv = obj.gets('a')) {
    r = obj.cas('a', 20, 30);
  }
  if(auto v = obj.get('a')) {
    BOOST_CHECK_EQUAL(*v, 2);
  }
  while(!r) {
    if(auto vv = obj.gets('a')) {
      r = obj.cas('a', 20, (*vv).second);
    }
  }
  if(auto v = obj.get('a')) {
    BOOST_CHECK_EQUAL(*v, 20);
  }

  paracel::kvs<char, std::vector<int> > obj2;
  std::vector<int> ttmp1 = {1, 2, 3};
  std::vector<int> ttmp2 = {3, 2, 1};
  obj2.set('a', ttmp1);
  obj2.incr('a', ttmp2);
  if(auto v = obj2.get('a')) {
    for(auto & item : *v) {
      BOOST_CHECK_EQUAL(item, 4);
    }
  }
  obj.incr('a', 30);
  if(auto v = obj.get('a')) {
    BOOST_CHECK_EQUAL(*v, 50);
  }
  obj.del('b'); obj.del('c');
  if(auto v = obj.get('b')) {
    BOOST_CHECK_EQUAL(*v, 7);
  }
  obj.clean();
  if(auto v = obj.get('a')) {
    BOOST_CHECK_EQUAL(*v, 7);
  }
}
