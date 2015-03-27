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
#define BOOST_TEST_MODULE RING_TEST 

#include <boost/test/unit_test.hpp>

#include <vector>
#include <string>
#include <fstream>
#include "ring.hpp"
#include "utils.hpp"

BOOST_AUTO_TEST_CASE (ring_test) {
  {
    std::vector<int> server_names{1, 2, 3};
    paracel::ring<int> ring(server_names);
    std::string key("dw");
    BOOST_CHECK_EQUAL(2, ring.get_server(key));
    std::string key2("q[,:0]_0");
    BOOST_CHECK_EQUAL(2, ring.get_server(key2));
    std::string key3("q[,:1]_0");
    BOOST_CHECK_EQUAL(3, ring.get_server(key3));
    std::string key4("q[,:2]_0");
    BOOST_CHECK_EQUAL(1, ring.get_server(key4));
    std::string key5("q[,:3]_0");
    BOOST_CHECK_EQUAL(3, ring.get_server(key5));
    std::string key6("p[0:,]_2");
    BOOST_CHECK_EQUAL(1, ring.get_server(key6));
    std::string key7("p[13:,]_2");
    BOOST_CHECK_EQUAL(ring.get_server(key7), 2);
    std::string key8("p[42:,]_2");
    BOOST_CHECK_EQUAL(3, ring.get_server(key8));
    std::string key9("p[5:,]_2");
    BOOST_CHECK_EQUAL(2, ring.get_server(key9));
    std::string key10("p[3:,]_1");
    BOOST_CHECK_EQUAL(3, ring.get_server(key10));
    // char* is unhashable
    //std::cout << ring.get_server("world") << std::endl;
  }
  {
    std::vector<std::string> server_names{"balin1", "beater5", "beater7"};
    paracel::ring<std::string> ring(server_names);
    std::string key("dw");
    BOOST_CHECK_EQUAL("beater7", ring.get_server(key));
    std::string key2("q[,:0]_0");
    BOOST_CHECK_EQUAL("beater5", ring.get_server(key2));
    std::string key3("q[,:1]_0");
    BOOST_CHECK_EQUAL("balin1", ring.get_server(key3));
    std::string key4("q[,:2]_0");
    BOOST_CHECK_EQUAL("beater7", ring.get_server(key4));
    std::string key5("q[,:3]_0");
    BOOST_CHECK_EQUAL("beater7", ring.get_server(key5));
    std::string key6("p[0:,]_2");
    BOOST_CHECK_EQUAL("balin1", ring.get_server(key6));
    std::string key7("p[13:,]_2");
    BOOST_CHECK_EQUAL("beater7", ring.get_server(key7));
    std::string key8("p[42:,]_2");
    BOOST_CHECK_EQUAL("balin1", ring.get_server(key8));
    std::string key9("p[5:,]_2");
    BOOST_CHECK_EQUAL("beater5", ring.get_server(key9));
    std::string key10("p[3:,]_1");
    BOOST_CHECK_EQUAL("beater5", ring.get_server(key10));
  }
}
