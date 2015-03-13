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
#define BOOST_TEST_MODULE CLIENT_TEST 

#include <boost/test/unit_test.hpp>

#include <string>
#include "utils.hpp"
#include "server.hpp"

BOOST_AUTO_TEST_CASE (server_test) {
  char hostname[1024];
  gethostname(hostname, sizeof(hostname));
  std::string server_name = hostname;
  paracel::init_thrds(server_name, "6378");
}
