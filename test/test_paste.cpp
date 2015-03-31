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
#define BOOST_TEST_MODULE PASTE_TEST 

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "utils/ext_utility.hpp"
#include "paracel_types.hpp"
#include "packer.hpp"

// terminate function for recursive variadic template
template<class T>
paracel::str_type paste(const T & arg) {
  paracel::packer<T> pk(arg);
  paracel::str_type scrip;
  pk.pack(scrip);
  return scrip;
}

// T must be paracel::str_type
// use template T to do recursive variadic
template<class T, class ...Args>
paracel::str_type paste(const T & op_str, const Args & ...args) { 
  paracel::packer<T> pk(op_str);
  paracel::str_type scrip;
  pk.pack(scrip); // pack to scrip
  return scrip + paracel::seperator + paste(args...); 
}

void check_result(const paracel::str_type & s) {
  auto result = paracel::str_split_by_word(s, paracel::seperator);
  std::cout << "--------------------------------" << std::endl;
  paracel::packer<paracel::str_type> pk;
  for(auto & s : result) {
    std::cout << "#" << pk.unpack(s) << "#" << std::endl;
  }
  std::cout << "--------------------------------" << std::endl;
}

BOOST_AUTO_TEST_CASE (paste_test) {
  {
    std::string key = "p[0,:]";
    std::string op_str = "pull";
    auto s1 = paste(op_str, key);
    check_result(s1);
    auto s2 = paste(std::string("pull"), key);
    std::cout << s2 << std::endl;
    check_result(s2);
    std::vector<std::string> key_lst = {"p[0,:]", "p[1,:]", "q[:,0]"};
    auto s3 = paste(std::string("pull_multi"), key_lst);
    //check_result(s3);
    auto result = paracel::str_split_by_word(s3, paracel::seperator);
    std::cout << "--------------------------------" << std::endl;
    paracel::packer<paracel::str_type> pk1;
    paracel::packer<paracel::list_type<paracel::str_type> > pk2;
    std::cout << "#" << pk1.unpack(result[0]) << "#" << std::endl;
    auto tmp = pk2.unpack(result[1]);
    for(auto & v : tmp)
      std::cout << "#" << v << "#" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    auto s4 = paste(std::string("pullall"));
    check_result(s4);
  }
  {
    std::string key = "q[:,0]";
    std::string v1 = "val";
    auto s1 = paste(std::string("push"), key, v1);
    check_result(s1);

    double v2 = 3.14;
    auto s2 = paste(std::string("push"), key, v2);
    auto result = paracel::str_split_by_word(s2, paracel::seperator);
    std::cout << "--------------------------------" << std::endl;
    paracel::packer<paracel::str_type> pk1;
    paracel::packer<double> pk2;
    std::cout << "#" << pk1.unpack(result[0]) << "#" << std::endl;
    std::cout << "#" << pk1.unpack(result[1]) << "#" << std::endl;
    std::cout << "#" << pk2.unpack(result[2]) << "#" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    
    std::vector<double> v3 = {1.1, 2.2, 3.3, 4.4, 5.5};
    auto s3 = paste(std::string("push"), key + "_0", v3);
    auto result2 = paracel::str_split_by_word(s3, paracel::seperator);
    std::cout << "--------------------------------" << std::endl;
    paracel::packer<std::vector<double> > pk3;
    std::cout << "#" << pk1.unpack(result2[0]) << "#" << std::endl;
    std::cout << "#" << pk1.unpack(result2[1]) << "#" << std::endl;
    auto tmp = pk3.unpack(result2[2]);
    for(auto & v : tmp) 
      std::cout << "#" << v << "#" << std::endl;
    std::cout << "--------------------------------" << std::endl;

    std::unordered_map<paracel::str_type, paracel::list_type<double> > d;
    paracel::list_type<double> t1 = {1., 2., 3., 4., 5.};
    paracel::list_type<double> t2 = {5., 4., 3., 2., 1.};
    d["p[0,:]_0"] = t1;
    d["q[:,0]_0"] = t2;
    auto s4 = paste(std::string("push_multi"), d);
    auto result3 = paracel::str_split_by_word(s4, paracel::seperator);

    std::cout << "--------------------------------" << std::endl;
    paracel::packer<std::unordered_map<paracel::str_type, paracel::list_type<double> > > pk4;
    std::cout << "#" << pk1.unpack(result3[0]) << "#" << std::endl;
    auto tmp2 = pk4.unpack(result3[1]);
    for(auto & v : tmp2) {
      std::cout << "#" << v.first << " : ";
      for(auto & va : v.second) {
        std::cout << va << ",";
      }
      std::cout << "#" << std::endl;
    }
    std::cout << "--------------------------------" << std::endl;
    
  }
  {
    std::string key = "q[:,0]";
    std::string v1 = "val";
    auto s1 = paste(std::string("remove"));
    check_result(s1);
  }
  {
    std::string key = "q[:,0]";
    std::string v1 = "val";
    auto s1 = paste(std::string("clear"));
    check_result(s1);
  }
  {
    double key = 3.14;
    int v1 = 7;
    std::string v2 = "hello";
    auto s1 = paste(std::string("push"), key, v1, v2);
    auto result = paracel::str_split_by_word(s1, paracel::seperator);
    std::cout << "--------------------------------" << std::endl;
    paracel::packer<paracel::str_type> pk0;
    paracel::packer<double> pk1;
    paracel::packer<int> pk2;
    std::cout << "#" << pk0.unpack(result[0]) << "#" << std::endl;
    std::cout << "#" << pk1.unpack(result[1]) << "#" << std::endl;
    std::cout << "#" << pk2.unpack(result[2]) << "#" << std::endl;
    std::cout << "#" << pk0.unpack(result[3]) << "#" << std::endl;
    std::cout << "--------------------------------" << std::endl;
  }
  {
    paracel::list_type<paracel::str_type> kl= {"k1", "k2", "k3"};
    paracel::list_type<int> vl = {1, 2, 3};
    auto result = paste(std::string("push_multi"), kl, vl);
    std::cout << result << std::endl;
    paracel::packer<paracel::list_type<paracel::str_type> > pk1;
    paracel::packer<paracel::list_type<int> > pk2;
    auto tmp = paracel::str_split_by_word(result, paracel::seperator);
    for(auto & v : pk1.unpack(tmp[1])) {
      std::cout << v << std::endl;
    }
    for(auto & v : pk2.unpack(tmp[2])) {
      std::cout << v << std::endl;
    }
  }
}
