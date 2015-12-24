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
#include "graph.hpp"

#define TOL 1e-7

#define PARACEL_CHECK_EQUAL(a, b) equal_check(a, b)
#define PARACEL_CHECK_CLOSE(a, b) close_check(a, b)

template <class T, class U>
void equal_check(T a, U b) {
  BOOST_CHECK_EQUAL(a, b);
}

template <class V>
void equal_check(std::vector<V> a,
                 std::vector<V> b) {
  BOOST_CHECK_EQUAL(a.size(), b.size());
  for(size_t i = 0; i < a.size(); ++i) {
    BOOST_CHECK_EQUAL(a[i], b[i]);
  }
}

template <class K, class V>
void equal_check(std::unordered_map<K, V> a,
                 std::unordered_map<K, V> b) {
  BOOST_CHECK_EQUAL(a.size(), b.size());
  for(auto & kv : a) {
    BOOST_CHECK(b.count(kv.first));
    BOOST_CHECK_EQUAL(kv.second, b[kv.first]);
  }
}

void equal_check(Eigen::MatrixXd a,
                 Eigen::MatrixXd b) {
  equal_check(paracel::mat2vec(a), paracel::mat2vec(b));
}

void equal_check(Eigen::SparseMatrix<double, Eigen::RowMajor> a,
                 Eigen::SparseMatrix<double, Eigen::RowMajor> b) {
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

template <class T>
void equal_check(paracel::undirected_graph<T> a,
                 paracel::undirected_graph<T> b) {
  BOOST_CHECK_EQUAL(a.v(), b.v());
  BOOST_CHECK_EQUAL(a.e(), b.e());
  BOOST_CHECK_EQUAL(a.degree(), b.degree());
  BOOST_CHECK_EQUAL(a.avg_degree(), b.avg_degree());
  BOOST_CHECK_EQUAL(a.max_degree(), b.max_degree());
  BOOST_CHECK_EQUAL(a.selfloops(), b.selfloops());
  PARACEL_CHECK_EQUAL(a.get_data(), b.get_data());
}

template <class T>
void equal_check(paracel::digraph<T> a,
                 paracel::digraph<T> b) {
  BOOST_CHECK_EQUAL(a.v(), b.v());
  BOOST_CHECK_EQUAL(a.e(), b.e());
  BOOST_CHECK_EQUAL(a.outdegree(), b.outdegree());
  BOOST_CHECK_EQUAL(a.indegree(), b.indegree());
  BOOST_CHECK_EQUAL(a.avg_degree(), b.avg_degree());
  BOOST_CHECK_EQUAL(a.selfloops(), b.selfloops());
  PARACEL_CHECK_EQUAL(a.get_data(), b.get_data());
}

template <class T>
void equal_check(paracel::bigraph<T> a,
                 paracel::bigraph<T> b) {
  BOOST_CHECK_EQUAL(a.v(), b.v());
  BOOST_CHECK_EQUAL(a.e(), b.e());
  BOOST_CHECK_EQUAL(a.avg_degree(), b.avg_degree());
  BOOST_CHECK_EQUAL(a.outdegree(), b.outdegree());
  BOOST_CHECK_EQUAL(a.indegree(), b.indegree());
  PARACEL_CHECK_EQUAL(a.get_data(), b.get_data());
}

void equal_check(paracel::bigraph_continuous a,
                 paracel::bigraph_continuous b) {
  BOOST_CHECK_EQUAL(a.v(), b.v());
  BOOST_CHECK_EQUAL(a.e(), b.e());
}

template <class T>
void close_check(T a, T b) {
  BOOST_CHECK_CLOSE(a, b, TOL);  
}

template <class V>
void close_check(std::vector<V> a,
                 std::vector<V> b) {
  BOOST_CHECK_EQUAL(a.size(), b.size());
  for(size_t i = 0; i < a.size(); ++i) {
    BOOST_CHECK_CLOSE(a[i], b[i], TOL);
  }
}

void close_check(Eigen::MatrixXd a,
                 Eigen::MatrixXd b) {
  close_check(paracel::mat2vec(a), paracel::mat2vec(b));
}

void close_check(Eigen::SparseMatrix<double, Eigen::RowMajor> a,
                 Eigen::SparseMatrix<double, Eigen::RowMajor> b) {
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
    BOOST_CHECK_CLOSE(w, std::get<2>(d[indx]), TOL);
    indx ++;
  }; 
  paracel::traverse_matrix(a, gen);
  paracel::traverse_matrix(b, checker);
}

#endif
