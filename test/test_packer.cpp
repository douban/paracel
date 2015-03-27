/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
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
#define BOOST_TEST_MODULE PACKER_TEST 

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <tuple>
#include <tr1/unordered_map>
#include <msgpack.hpp>

#include "paracel_types.hpp"
#include "packer.hpp"
#include "graph.hpp"

struct AA {
 public:
  AA() : a(1), b(1.) {}
  AA(int i, double j) : a(i), b(j) {}
  ~AA() {}
  void dump() {
    std::cout << "a: " << a << " b: " << b << std::endl;
  }
  int a;
  double b;
  MSGPACK_DEFINE(a, b);
};

BOOST_AUTO_TEST_CASE (packer_test) {
  {
    paracel::packer<bool> obj(true);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_EQUAL(r, 1);
  }
  {
    paracel::packer<bool> obj(false);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_EQUAL(r, 0);
  }
  {
    paracel::packer<int> obj(54);
    std::string s;
    obj.pack(s);
    std::cout << s << std::endl;
    auto r = obj.unpack(s);
    BOOST_CHECK_EQUAL(r, 54);
  }
  {
    paracel::packer<int> obj(51);
    msgpack::sbuffer s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_EQUAL(r, 51);
  }
  {
    paracel::list_type<paracel::str_type> target = {"hello", "world"};
    paracel::packer<paracel::list_type<paracel::str_type> > obj(target);
    msgpack::sbuffer s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_EQUAL("hello", r[0]);
    BOOST_CHECK_EQUAL("world", r[1]);
  }
  {
    paracel::str_type target = "PARACEL";
    paracel::packer<paracel::str_type> obj(target);
    msgpack::sbuffer s;
    obj.pack(s);
    BOOST_CHECK_EQUAL("PARACEL", obj.unpack(s));
  }
  {
    paracel::list_type<paracel::str_type> target = {"hello", "world"};
    paracel::packer<paracel::list_type<paracel::str_type> > obj(target);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_EQUAL("hello", r[0]);
    BOOST_CHECK_EQUAL("world", r[1]);
  }
  {
    paracel::str_type target("test,data");
    paracel::packer<> obj(target);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_EQUAL(r, "test,data");
  }
  {
    double target = 3.14;
    paracel::packer<double> obj(target);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_EQUAL(r, 3.14);
  }
  {
    paracel::list_type<int> target = {77, 88};
    paracel::packer<paracel::list_type<int> > obj(target);
    std::string s;
    obj.pack(s);
    std::cout << s << std::endl;
    auto r = obj.unpack(s);
    BOOST_CHECK_EQUAL(77, r[0]);
    BOOST_CHECK_EQUAL(88, r[1]);
  }
  {
    paracel::list_type<double> target = {1., 2., 3.};
    paracel::packer<paracel::list_type<double> > obj(target);
    msgpack::sbuffer s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_LT(1. - r[0], 0.0000001);
    BOOST_CHECK_LT(2. - r[1], 0.0000001);
    BOOST_CHECK_LT(3. - r[2], 0.0000001);
  }
  {
    paracel::list_type<double> target = {1., 2., 3.};
    paracel::packer<paracel::list_type<double> > obj(target);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_LT(1. - r[0], 0.0000001);
    BOOST_CHECK_LT(2. - r[1], 0.0000001);
    BOOST_CHECK_LT(3. - r[2], 0.0000001);
  }
  {
    paracel::list_type<float> target = {1.1, 2.2, 3.3};
    paracel::packer<paracel::list_type<float> > obj(target);
    msgpack::sbuffer s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_LT(1.1 - r[0], 0.000001);
    BOOST_CHECK_LT(2.2 - r[1], 0.000001);
    BOOST_CHECK_LT(3.3 - r[2], 0.000001);
  }
  {
    paracel::list_type<float> target = {1.1, 2.2, 3.3};
    paracel::packer<paracel::list_type<float> > obj(target);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    BOOST_CHECK_LT(1.1 - r[0], 0.000001);
    BOOST_CHECK_LT(2.2 - r[1], 0.000001);
    BOOST_CHECK_LT(3.3 - r[2], 0.000001);
  }
  {
    std::map<paracel::str_type, paracel::list_type<float> > d;
    paracel::list_type<float> target1 = {1.1, 2.2, 3.3};
    paracel::list_type<float> target2 = {3.3, 2.2, 1.1};
    d["key_0"] = target1;
    d["key_1"] = target2;
    paracel::packer<std::map<paracel::str_type, paracel::list_type<float> > > obj(d);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      for(auto & va : v.second) {
        std::cout << va << std::endl;
      }
    }
  }
  {
    std::tr1::unordered_map<int, int> d;
    d[1] = 1;
    d[2] = 2;
    paracel::packer<std::tr1::unordered_map<int, int> > obj(d);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
        std::cout << "#" << v.first << ":" << v.second << "#" << std::endl;
    }
  }
  {
    std::tr1::unordered_map<paracel::str_type, paracel::list_type<float> > d;
    paracel::list_type<float> target1 = {1.1, 2.2, 3.3};
    paracel::list_type<float> target2 = {3.3, 2.2, 1.1};
    d["key_0"] = target1;
    d["key_1"] = target2;
    paracel::packer<std::tr1::unordered_map<paracel::str_type, paracel::list_type<float> > > obj(d);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      for(auto & va : v.second) {
        std::cout << va << std::endl;
      }
    }
  }
  {
    std::tr1::unordered_map<paracel::str_type, paracel::list_type<double> > d;
    paracel::list_type<double> target1 = {1.11, 2.22, 3.33};
    paracel::list_type<double> target2 = {3.33, 2.22, 1.11};
    d["key_0"] = target1;
    d["key_1"] = target2;
    paracel::packer<std::tr1::unordered_map<paracel::str_type, paracel::list_type<double> > > obj(d);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      for(auto & va : v.second) {
        std::cout << va << std::endl;
      }
    }
  }
  {
    std::unordered_map<paracel::str_type, paracel::list_type<double> > d;
    paracel::list_type<double> target1 = {7.77, 8.88, 9.99};
    paracel::list_type<double> target2 = {9.99, 8.88, 7.77};
    d["key_0"] = target1;
    d["key_1"] = target2;
    paracel::packer<std::unordered_map<paracel::str_type, paracel::list_type<double> > > obj(d);
    std::string s;
    obj.pack(s);
    auto r = obj.unpack(s);
    for(auto & v : r) {
      for(auto & va : v.second) {
        std::cout << va << std::endl;
      }
    }
  }
  {
    AA d(7, 3.14);
    paracel::packer<AA> obj(d);
    std::string s;
    obj.pack(s);
    AA r = obj.unpack(s);
    r.dump();
  }
  {
    paracel::list_type<std::tuple<size_t, size_t, double> > tpls;
    tpls.emplace_back(std::make_tuple(0, 0, 3.));
    tpls.emplace_back(std::make_tuple(0, 2, 5.));
    tpls.emplace_back(std::make_tuple(1, 0, 4.));
    tpls.emplace_back(std::make_tuple(1, 1, 3.));
    tpls.emplace_back(std::make_tuple(1, 2, 1.));
    tpls.emplace_back(std::make_tuple(2, 0, 2.));
    tpls.emplace_back(std::make_tuple(2, 3, 1.));
    tpls.emplace_back(std::make_tuple(3, 1, 3.));
    tpls.emplace_back(std::make_tuple(3, 3, 1.));
    paracel::digraph<size_t> bgrp(tpls);
    BOOST_CHECK_EQUAL(4, bgrp.v());
    BOOST_CHECK_EQUAL(9, bgrp.e());
    BOOST_CHECK_EQUAL(2, bgrp.avg_degree());
    auto kk = bgrp.adjacent(0);
    BOOST_CHECK_EQUAL(2, kk.size());

    paracel::packer<paracel::digraph<size_t> > obj(bgrp);
    std::string s;
    obj.pack(s);
    paracel::digraph<size_t> r = obj.unpack(s);
    BOOST_CHECK_EQUAL(4, r.v());
    BOOST_CHECK_EQUAL(9, r.e());
    BOOST_CHECK_EQUAL(2, r.avg_degree());
    auto kkk = r.adjacent(0);
    BOOST_CHECK_EQUAL(kkk.size(), 2);
  }
}
