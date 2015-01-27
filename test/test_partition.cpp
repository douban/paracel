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
#define BOOST_TEST_MODULE PARTITION_TEST 

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include "load/partition.hpp"
#include "utils.hpp"
#include "test.hpp"
#include "paracel_types.hpp"

const std::string fname = "test_partition.dat";

void create_testfile(int line_sz) {
  std::ofstream os;
  os.open(fname);
  for(int i = 0; i < line_sz; ++i) {
    os << "1234567 " << std::to_string(i % 10) << '\n';
  }
  os.close();
}

BOOST_AUTO_TEST_CASE (files_partition_test) {
  int lines = 10000;
  create_testfile(lines);
  std::vector<std::string> flst = {fname};
  {
    for(int np = 1; np < 100; ++np) {
      
      paracel::partition obj(flst, np, "fmap");
      std::vector<long> ss, ee;
      obj.file_partition(fname, ss, ee);

      obj.files_partition(1);
      std::vector<long> ss_check = obj.get_start_list();
      std::vector<long> ee_check = obj.get_end_list();
      
      BOOST_CHECK_EQUAL_V(ss, ss_check);
      BOOST_CHECK_EQUAL_V(ee, ee_check);
      BOOST_CHECK_EQUAL(ss.size(), ee.size());
      BOOST_CHECK_EQUAL(ss_check.size(), ee_check.size());
      
      for(size_t i = 0; i < ss.size(); ++i) {
        auto lines = obj.file_load_lines_impl(fname, ss[i], ee[i]);
        auto lines_check = obj.files_load_lines_impl(ss_check[i], ee_check[i]);
        for(size_t j = 0; j < lines.size() - 1; ++j) {
          int k1 = lines[j+1].back() - '0', k2 = lines[j].back() - '0';
          if(k1 == 0 && k2 == 9) {
            continue;
          } else {
            BOOST_CHECK_EQUAL(k1 - k2, 1);
          }
        }
        BOOST_CHECK_EQUAL_V(lines, lines_check);
      }

    } // np
  }
}
