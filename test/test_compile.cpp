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
#define BOOST_TEST_MODULE COMPILE_TEST 

#include <boost/test/unit_test.hpp>

#include "ps.hpp"
#include "client.hpp"
#include "server.hpp"
#include "load.hpp"
#include "graph.hpp"
#include "kv.hpp"
#include "kv_def.hpp"
#include "ring.hpp"
#include "utils.hpp"
#include "proxy.hpp"
#include "packer.hpp"
#include "paracel_types.hpp"
#include "pregel.hpp"

BOOST_AUTO_TEST_CASE (compile_test) {
  BOOST_CHECK(true);
}
