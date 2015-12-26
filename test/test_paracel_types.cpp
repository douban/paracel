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
#define BOOST_TEST_MODULE PARACEL_TYPES_TEST 

#include <boost/test/unit_test.hpp>
#include <vector>
#include <string>
#include <iostream>
#include "paracel_types.hpp"
#include "test.hpp"

BOOST_AUTO_TEST_CASE (paracel_types_test) {
  PARACEL_CHECK_EQUAL(true, paracel::is_atomic<int>::value);
  PARACEL_CHECK_EQUAL(false, paracel::is_atomic<char>::value);
  PARACEL_CHECK_EQUAL(false, paracel::is_atomic<std::vector<int>>::value);
  PARACEL_CHECK_EQUAL(true, paracel::is_seqic<std::vector<int>>::value);
  PARACEL_CHECK_EQUAL(true, paracel::is_seqic<std::vector<double>>::value);

  paracel::hash_type<std::string> hfunc;
  PARACEL_CHECK_EQUAL(hfunc("abcd"), 16030370620903852840UL);

  std::vector<int> tmp2{1, 2, 3};
  paracel::hash_type< std::vector<int> > csfunc;
  PARACEL_CHECK_EQUAL(csfunc(tmp2), 15152834183934801446UL);

  std::vector<char> tmp3{'a', 'b', 'c'};
  paracel::hash_type< std::vector<char> > csfunc2;
  PARACEL_CHECK_EQUAL(csfunc2(tmp3), 1227721600474941649UL);

  paracel::bag_type<int> bg;
  bg.put(1); bg.put(3); bg.put(5);
  auto bg_lambda = [] (int item) { 
    std::cout << item << std::endl;
  };
  bg.traverse(bg_lambda);
  auto bg_cp = bg.get();
  PARACEL_CHECK_EQUAL(3, static_cast<int>(bg_cp.size()));
  bg_cp.traverse(bg_lambda);

  paracel::bag_type<int> bg2(bg);
  bg2.traverse(bg_lambda);

  for(auto & i : bg2) {
    std::cout << i << std::endl;
  }
}
