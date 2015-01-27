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

#ifndef FILE_58f9fa9e_188f_5f43_d103_c16c3132fd24_HPP
#define FILE_58f9fa9e_188f_5f43_d103_c16c3132fd24_HPP

#include <vector>
#include <unordered_map>
#include <boost/test/unit_test.hpp>

#define BOOST_CHECK_EQUAL_V(a, b) equal_check_v(a, b)
#define BOOST_CHECK_EQUAL_D(a, b) equal_check_d(a, b)

template <class V>
void equal_check_v(std::vector<V> a,
                   std::vector<V> b) {
  BOOST_CHECK_EQUAL(a.size(), b.size());
  for(size_t i = 0; i < a.size(); ++i) {
    BOOST_CHECK_EQUAL(a[i], b[i]);
  }
}

template <class K, class V>
void equal_check_d(std::unordered_map<K, V> a,
                   std::unordered_map<K, V> b) {
  BOOST_CHECK_EQUAL(a.size(), b.size());
  for(auto & kv : a) {
    BOOST_CHECK(b.count(kv.first));
    BOOST_CHECK_EQUAL(kv.second, b[kv.first]);
  }
}

#endif
