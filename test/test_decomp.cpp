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
#define BOOST_TEST_MODULE DECOMP_TEST 
#include <boost/test/unit_test.hpp>
#include "utils/decomp.hpp"
using namespace paracel;
BOOST_AUTO_TEST_CASE (decomp_test) {
  int x, y; 
  npfact2d(10, x, y);
  BOOST_CHECK_EQUAL(x, 2);
  BOOST_CHECK_EQUAL(y, 5);
  npfact2d(10, x, y, false);
  BOOST_CHECK_EQUAL(x, 5);
  BOOST_CHECK_EQUAL(y, 2);
  npfact2d(7, x, y);
  BOOST_CHECK_EQUAL(x, 7);
  BOOST_CHECK_EQUAL(y, 1);
  npfact2d(7, x, y, false);
  BOOST_CHECK_EQUAL(x, 1);
  BOOST_CHECK_EQUAL(y, 7);
  npfact2d(2, x, y);
  BOOST_CHECK_EQUAL(x, 2);
  BOOST_CHECK_EQUAL(y, 1);
  npfact2d(6, x, y);
  BOOST_CHECK_EQUAL(x, 2);
  BOOST_CHECK_EQUAL(y, 3);
  npfact2d(8, x, y);
  BOOST_CHECK_EQUAL(x, 2);
  BOOST_CHECK_EQUAL(y, 4);
}
