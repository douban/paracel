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
#define BOOST_TEST_MODULE UTILS_TEST 

#include <vector>
#include <string>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include "utils.hpp"
#include "test.hpp"

BOOST_AUTO_TEST_CASE (json_parser_test) {
  paracel::json_parser pt("../../test/test.json");
  std::string r = "hong";

  BOOST_CHECK_EQUAL(pt.parse<std::string>("wu"), r);
  BOOST_CHECK_EQUAL(pt.parse<int>("hong"), 7);
  BOOST_CHECK_EQUAL(pt.parse<bool>("changsheng"), true);
  BOOST_CHECK_EQUAL(pt.parse<double>("jiang"), 3.141592653);
  std::vector<std::string> vl1 = {"hong", "xun", "zhang"};
  BOOST_CHECK_EQUAL_V(pt.parse_v<std::string>("wul"), vl1);
  std::vector<int> vl2 = {1, 2, 3, 4, 5, 6, 7};
  BOOST_CHECK_EQUAL_V(pt.parse_v<int>("hongl"), vl2);
  std::vector<bool> vl3 = {true, false, false, true, true};
  BOOST_CHECK_EQUAL_V(pt.parse_v<bool>("changshengl"), vl3);
  std::vector<double> vl4 = {1.23, 2.34, 3.45, 4.56, 5.67, 6.78, 7.89};
  BOOST_CHECK_EQUAL_V(pt.parse_v<double>("jiangl"), vl4);
}
