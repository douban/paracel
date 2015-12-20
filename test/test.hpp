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
#include <tuple>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include <boost/test/unit_test.hpp>
#include "utils.hpp"

#define BOOST_CHECK_EQUAL_V(a, b) equal_check_v(a, b)
#define BOOST_CHECK_EQUAL_D(a, b) equal_check_d(a, b)
#define BOOST_CHECK_EQUAL_M(a, b) equal_check_m(a, b)

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

void equal_check_m(Eigen::MatrixXd & a, Eigen::MatrixXd & b) {
  BOOST_CHECK_EQUAL_V(paracel::mat2vec(a), paracel::mat2vec(b));
}

void equal_check_m(Eigen::SparseMatrix<double, Eigen::RowMajor> & a,
                   Eigen::SparseMatrix<double, Eigen::RowMajor> & b) {
  BOOST_CHECK_EQUAL(a.rows(), b.rows());
  BOOST_CHECK_EQUAL(a.cols(), b.cols());
  std::vector<std::tuple<int, int, double>> d;
  auto gen = [&] (int i, int j, double w) {
    d.push_back(std::make_tuple(i, j, w));
  };
  size_t indx = 0;
  auto checker = [&] (int i, int j, double w) {
    BOOST_CHECK_EQUAL(i, std::get<0>(d[indx]));
    BOOST_CHECK_EQUAL(j, std::get<1>(d[indx]));
    BOOST_CHECK_EQUAL(w, std::get<2>(d[indx]));
    indx ++;
  }; 
  paracel::traverse_matrix(a, gen);
  paracel::traverse_matrix(b, checker);
}

#endif
