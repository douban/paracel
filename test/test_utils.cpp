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
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/QR>
#include <eigen3/Eigen/Cholesky>
#include <boost/test/unit_test.hpp>
#include "utils.hpp"
#include "test.hpp"
#include "graph.hpp"
#include "paracel_types.hpp"

BOOST_AUTO_TEST_CASE (paracel_str_extra_test) {
  {
    std::vector<std::string> init_lst = {"hello", "world", "happy", "new", "year", "2015"};
    std::string seps = "orz";
    auto together = paracel::str_join(init_lst, seps);
    BOOST_CHECK_EQUAL(together, "helloorzworldorzhappyorzneworzyearorz2015");
    auto res1 = paracel::str_split_by_word(together, seps);
    std::string tmp = together;
    auto res2 = paracel::str_split_by_word(std::move(tmp), seps);
    BOOST_CHECK_EQUAL_V(res1, init_lst);
    BOOST_CHECK_EQUAL_V(res2, init_lst);
    BOOST_CHECK_EQUAL(paracel::startswith(together, "hello"), true);
    BOOST_CHECK_EQUAL(paracel::startswith(together, "helo"), false);
    BOOST_CHECK_EQUAL(paracel::startswith(together, "helloorzworldorzhappyorzneworzyearorz2015"), true);
    BOOST_CHECK_EQUAL(paracel::startswith(together, "helloorzworldorzhappyorzneworzyearorz20157"), false);
    BOOST_CHECK_EQUAL(paracel::startswith(together, ""), true);
    BOOST_CHECK_EQUAL(paracel::endswith(together, "2015"), true);
    BOOST_CHECK_EQUAL(paracel::endswith(together, "2014"), false);
    BOOST_CHECK_EQUAL(paracel::endswith(together, "helloorzworldorzhappyorzneworzyearorz2015"), true);
    BOOST_CHECK_EQUAL(paracel::endswith(together, "7helloorzworldorzhappyorzneworzyearorz2015"), false);
    BOOST_CHECK_EQUAL(paracel::endswith(together, ""), true);
  }
  {
    std::string tmp = "a|b|c|d|e";
    std::vector<std::string> r = {"a", "b", "c", "d", "e"};
    auto r1 = paracel::str_split(tmp, '|');
    auto r2 = paracel::str_split(tmp, "|");
    auto r3 = paracel::str_split(tmp, "?|?");
    BOOST_CHECK_EQUAL_V(r1, r);
    BOOST_CHECK_EQUAL_V(r2, r);
    BOOST_CHECK_EQUAL_V(r3, r);
  }
}

BOOST_AUTO_TEST_CASE (paracel_random_test) {
  int incycle_cnt = 0;
  for(int i = 0; i < 10000000; ++i) {
    double x = paracel::random_double();
    double y = paracel::random_double();
    if(x * x + y * y < 1.)  incycle_cnt += 1;
  }
  std::cout << 4 * static_cast<double>(incycle_cnt) / 10000000. << std::endl;

  for(int i = 0; i < 1000000; ++i) auto r = paracel::random_double_list(100);

  auto r = paracel::random_double_list(1000000);
}

BOOST_AUTO_TEST_CASE (paracel_json_parser_test) {
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

BOOST_AUTO_TEST_CASE (utils_hash_test) {
  paracel::hash_type<paracel::default_id_type> hfunc;
  paracel::default_id_type a = 0, b = 1, c = 2, d = 3;
  BOOST_CHECK_EQUAL(hfunc(a), 0);
  BOOST_CHECK_EQUAL(hfunc(b), 1);
  BOOST_CHECK_EQUAL(hfunc(c), 2);
  BOOST_CHECK_EQUAL(hfunc(d), 3);
  paracel::hash_type<std::string> hfunc2;
  std::string x = "0", y = "1", z = "2", t = "3";
  a = 2297668033614959926ULL;
  b = 10159970873491820195ULL;
  c = 4551451650890805270ULL;
  d = 8248777770799913213ULL;
  BOOST_CHECK_EQUAL(hfunc2(x), a);
  BOOST_CHECK_EQUAL(hfunc2(y), b);
  BOOST_CHECK_EQUAL(hfunc2(z), c);
  BOOST_CHECK_EQUAL(hfunc2(t), d);
}

BOOST_AUTO_TEST_CASE (eigen_common_usage_test) {
  {
    // Eigen::Matrix is static type while Eigen::MatrixXd is dynamic
    Eigen::Matrix<double, 3, 3, Eigen::RowMajor> m = Eigen::Matrix3d::Identity();
    m.col(1) = Eigen::Vector3d(4, 5, 6);
    BOOST_CHECK_EQUAL(m.row(0)[0], 1);
    BOOST_CHECK_EQUAL(m.row(0)[1], 4);
    BOOST_CHECK_EQUAL(m.row(0)[2], 0);
    BOOST_CHECK_EQUAL(m.row(1)[0], 0);
    BOOST_CHECK_EQUAL(m.row(1)[1], 5);
    BOOST_CHECK_EQUAL(m.row(1)[2], 0);
    BOOST_CHECK_EQUAL(m.row(2)[0], 0);
    BOOST_CHECK_EQUAL(m.row(2)[1], 6);
    BOOST_CHECK_EQUAL(m.row(2)[2], 1);
    BOOST_CHECK_EQUAL(m.IsRowMajor, true);

    auto v = paracel::mat2vec(m);
    std::vector<double> check_v = {1, 0, 0, 4, 5, 6, 0, 0, 1};
    BOOST_CHECK_EQUAL_V(v, check_v);
    Eigen::MatrixXd check_mat = paracel::vec2mat(v, 3);
    Eigen::VectorXd vv = m.col(1);
    auto vvv = paracel::evec2vec(vv);
    std::vector<double> check_vvv = {4, 5, 6};
    BOOST_CHECK_EQUAL_V(vvv, check_vvv);
    Eigen::VectorXd vvvv = paracel::vec2evec(vvv);

    Eigen::Vector3d vec(1, 2, 3);
    auto r = m.row(2) * vec;
    BOOST_CHECK_EQUAL(r, 15);

    srand( (unsigned)time(NULL) );
    Eigen::Matrix3d mm = Eigen::Matrix3d::Random();
    std::cout << "matrix mm:" << std::endl << mm << std::endl;
    std::cout << "sum of each column:" << std::endl << mm.colwise().sum() << std::endl;
    std::cout << "sum of each row:" << std::endl << mm.rowwise().sum() << std::endl;
    std::cout << "maximum absolute value of each column:" 
        << std::endl << mm.cwiseAbs().colwise().maxCoeff() << std::endl;
  }

  {
    Eigen::MatrixXd mtx(3, 2);
    Eigen::VectorXd vech(2);
    vech[0] = 1.1; vech[1] = 0.8;
    mtx.row(0) = vech;
    vech[0] = 2.; vech[1] = 2.;
    mtx.row(1) = vech;
    vech[0] = 1.; vech[1] = 0.5;
    mtx.row(2) = vech;
    vech[0] = 1.01; vech[1] = 0.28;
    BOOST_CHECK_EQUAL(vech[0], 1.01);
    BOOST_CHECK_EQUAL(vech[1], 0.28);
    BOOST_CHECK_EQUAL(mtx.col(0)[0], 1.1);
    BOOST_CHECK_EQUAL(mtx.col(0)[1], 2);
    BOOST_CHECK_EQUAL(mtx.col(0)[2], 1);
    BOOST_CHECK_EQUAL(mtx.col(1)[0], 0.8);
    BOOST_CHECK_EQUAL(mtx.col(1)[1], 2);
    BOOST_CHECK_EQUAL(mtx.col(1)[2], 0.5);

    Eigen::MatrixXd::Index indx;
    (mtx.rowwise() - vech.transpose()).rowwise().squaredNorm().minCoeff(&indx);
    BOOST_CHECK_EQUAL(indx, 2);

    auto result = mtx * vech; 
    BOOST_CHECK_EQUAL(result.row(0)[0], 1.3350000000000002);
    BOOST_CHECK_EQUAL(result.row(1)[0], 2.58);
    BOOST_CHECK_EQUAL(result.row(2)[0], 1.15);

    mtx.row(0) *= 10;
    BOOST_CHECK_EQUAL(mtx.row(0)[0], 11);
    BOOST_CHECK_EQUAL(mtx.row(0)[1], 8);

    Eigen::Matrix2d mmat;
    mmat << 1, 2, 
         3, 4; 
    Eigen::VectorXd vvec(2);
    int rr = -1, cc = -1;
    vvec << 1.1, 2.2;
    BOOST_CHECK_EQUAL(mmat.maxCoeff(&rr, &cc), 4);
    BOOST_CHECK_EQUAL(rr, 1);
    BOOST_CHECK_EQUAL(cc, 1);
    BOOST_CHECK_EQUAL(mmat.row(1).sum(), 7);
  }
}

BOOST_AUTO_TEST_CASE (eigen_matrix_usage_test) {

  Eigen::MatrixXd HH_global = Eigen::MatrixXd::Random(3, 3);
  std::cout << HH_global << std::endl;
  Eigen::MatrixXd HH_blk = HH_global.block(0, 0, 2, 2);
  std::cout << HH_blk << std::endl;
  std::vector<double> tHt = paracel::mat2vec(HH_blk);
  std::cout << paracel::vec2mat(tHt, 2) << std::endl;

  Eigen::MatrixXd mtx(4, 3);
  mtx << 1., 2., 3.,
      4., 5., 6.,
      7., 8., 9.,
      10., 11., 12.;
  Eigen::MatrixXd clusters_mtx(2, 3);
  std::vector<double> c1 = {2.5, 3.5, 4.5}, c2 = {8.5, 9.5, 10.5};
  clusters_mtx.row(0) = paracel::vec2evec(c1);
  clusters_mtx.row(1) = paracel::vec2evec(c2);

  for(size_t i = 0; i < (size_t)mtx.rows(); ++i) {
    Eigen::MatrixXd::Index indx;
    (clusters_mtx.rowwise() - mtx.row(i)).rowwise().squaredNorm().minCoeff(&indx);
  }
  typedef Eigen::Triplet<double> eigen_triple;
  std::vector<eigen_triple> tpls;
  tpls.push_back(eigen_triple(0, 0, 0.6));
  tpls.push_back(eigen_triple(0, 2, 0.7));
  tpls.push_back(eigen_triple(0, 4, 0.4));
  tpls.push_back(eigen_triple(1, 2, 0.6));
  tpls.push_back(eigen_triple(1, 3, 0.5));
  tpls.push_back(eigen_triple(1, 4, 0.3));
  tpls.push_back(eigen_triple(2, 0, 0.3));
  tpls.push_back(eigen_triple(2, 1, 0.1));
  tpls.push_back(eigen_triple(3, 3, 0.1));
  tpls.push_back(eigen_triple(3, 4, 0.7));
  tpls.push_back(eigen_triple(4, 1, 0.3));
  tpls.push_back(eigen_triple(5, 0, 0.1));
  tpls.push_back(eigen_triple(5, 4, 0.7));
  tpls.push_back(eigen_triple(6, 0, 0.2));
  tpls.push_back(eigen_triple(6, 2, 0.8));
  tpls.push_back(eigen_triple(7, 0, 0.3));
  tpls.push_back(eigen_triple(8, 1, 0.1));
  tpls.push_back(eigen_triple(8, 2, 0.2));
  tpls.push_back(eigen_triple(8, 3, 0.3));
  tpls.push_back(eigen_triple(8, 4, 0.4));
  tpls.push_back(eigen_triple(9, 0, 0.9));
  tpls.push_back(eigen_triple(9, 3, 0.1));
  tpls.push_back(eigen_triple(9, 4, 0.2));
  Eigen::SparseMatrix<double, Eigen::RowMajor> A;
  A.resize(10, 5);
  A.setFromTriplets(tpls.begin(), tpls.end());
  auto vecA = paracel::mat2vec(A);
  BOOST_CHECK_EQUAL_M(A, paracel::vec2mat(vecA));
  Eigen::MatrixXd H(5, 3); // 5 * 3
  H << 1., 2., 3.,
    4., 5., 6.,
    7., 8., 9.,
    10., 11., 12.,
    13., 14., 15.;
  Eigen::MatrixXd W(1,1);
  W.resize(10, 3);
  W = A * H;
  std::cout << W << std::endl;

  Eigen::HouseholderQR<Eigen::MatrixXd> qr(W.transpose() * W);
  Eigen::MatrixXd squareW(2, 2);
  squareW << 1., 2., 3., 4.;
  std::cout << squareW.inverse() << std::endl;
  Eigen::MatrixXd R = qr.matrixQR().triangularView<Eigen::Upper>();
  std::cout << R << std::endl;
  std::cout << R.inverse() << std::endl;

  Eigen::MatrixXd AA(3, 3);
  AA <<  0.872871, -0.574833, -0.304016,
     -0.574833, 0.827987, 0.120067,
     -0.304016, 0.120067, 0.297787;
  std::cout << "The matrix AA is" << std::endl << AA << std::endl;
  Eigen::LLT<Eigen::MatrixXd> lltOfA(AA); // compute the Cholesky decomposition of A
  Eigen::MatrixXd L = lltOfA.matrixL(); // retrieve factor L  in the decomposition

  // The previous two lines can also be written as "L = A.llt().matrixL()"
  std::cout << "The Cholesky factor L is" << std::endl << -L << std::endl;
  std::cout << "To check this, let us compute L * L.transpose()" << std::endl;
  std::cout << L * L.transpose() << std::endl;
  std::cout << "This should equal the matrix A" << std::endl;

  Eigen::MatrixXf ma = Eigen::MatrixXf::Random(3, 2);
  std::cout << "Here is the matrix m:" << std::endl << ma << std::endl;
  Eigen::JacobiSVD<Eigen::MatrixXf> svd(ma, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::MatrixXf SIGMA = svd.singularValues();
  Eigen::MatrixXf U = svd.matrixU();
  Eigen::MatrixXf V = svd.matrixV();
  std::cout << SIGMA.rows() << SIGMA.cols() << U.rows() << U.cols()
      << V.rows() << V.cols() << std::endl;
  Eigen::MatrixXf sigmaMat = Eigen::MatrixXf::Zero(ma.rows(), ma.cols());
  sigmaMat.diagonal() = SIGMA;

  std::cout << U * sigmaMat * V.transpose() << std::endl;

  std::cout << "Its singular values are:" << std::endl << SIGMA << std::endl;
  std::cout << "Its left singular vectors are columns of the thin U matrix:" << std::endl << U << std::endl;
  std::cout << "Its right singular vectors are columns of the thin V matrix:" << std::endl << V << std::endl;

  std::cout.precision(20);
  Eigen::MatrixXd H_blk(4, 3);
  Eigen::MatrixXd stone = Eigen::MatrixXd::Random(2, 3);
  H_blk.row(0) = stone.row(0);
  H_blk.row(1) = stone.row(1);
  H_blk.row(2) = stone.row(0);
  H_blk.row(3) = stone.row(1);
  std::cout << stone.transpose() * stone << std::endl;
  auto kk = H_blk.block(0, 0, 2, 3).transpose() * H_blk.block(0, 0, 2, 3);
  auto kk2 = H_blk.block(2, 0, 2, 3).transpose() * H_blk.block(2, 0, 2, 3);
  Eigen::MatrixXd tt(3, 3);
  tt << 
      kk.row(0)[1] + kk2.row(0)[1],
      kk.row(0)[1] + kk2.row(0)[1],
      kk.row(0)[2] + kk2.row(0)[2],
      kk.row(1)[0] + kk2.row(1)[0],
      kk.row(1)[1] + kk2.row(1)[1],
      kk.row(1)[2] + kk2.row(1)[2],
      kk.row(2)[0] + kk2.row(2)[0],
      kk.row(2)[1] + kk2.row(2)[1],
      kk.row(2)[2] + kk2.row(2)[2];

  std::cout << tt << std::endl;
  std::cout << tt.inverse() << std::endl;
  std::cout << (H_blk.transpose() * H_blk).inverse() << std::endl;
}

BOOST_AUTO_TEST_CASE (pkl_sequential_usage_test) {
  { // sparse matrix
    typedef Eigen::Triplet<double> eigen_triple;
    std::vector<eigen_triple> tpls;
    tpls.push_back(eigen_triple(0, 0, 0.6));
    tpls.push_back(eigen_triple(0, 2, 0.7));
    tpls.push_back(eigen_triple(0, 4, 0.4));
    tpls.push_back(eigen_triple(1, 2, 0.6));
    tpls.push_back(eigen_triple(1, 3, 0.5));
    tpls.push_back(eigen_triple(1, 4, 0.3));
    tpls.push_back(eigen_triple(2, 0, 0.3));
    tpls.push_back(eigen_triple(2, 1, 0.1));
    tpls.push_back(eigen_triple(3, 3, 0.1));
    tpls.push_back(eigen_triple(3, 4, 0.7));
    tpls.push_back(eigen_triple(4, 1, 0.3));
    tpls.push_back(eigen_triple(5, 0, 0.1));
    tpls.push_back(eigen_triple(5, 4, 0.7));
    tpls.push_back(eigen_triple(6, 0, 0.2));
    tpls.push_back(eigen_triple(6, 2, 0.8));
    tpls.push_back(eigen_triple(7, 0, 0.3));
    tpls.push_back(eigen_triple(8, 1, 0.1));
    tpls.push_back(eigen_triple(8, 2, 0.2));
    tpls.push_back(eigen_triple(8, 3, 0.3));
    tpls.push_back(eigen_triple(8, 4, 0.4));
    tpls.push_back(eigen_triple(9, 0, 0.9));
    tpls.push_back(eigen_triple(9, 3, 0.1));
    tpls.push_back(eigen_triple(9, 4, 0.2));
    Eigen::SparseMatrix<double, Eigen::RowMajor> A, Arev;
    A.resize(10, 5);
    A.setFromTriplets(tpls.begin(), tpls.end());
    paracel::pkl_dat_sequential(A, "/var/tmp/test.pkl");
    paracel::unpkl_dat_sequential("/var/tmp/test.pkl", Arev);
    BOOST_CHECK_EQUAL_M(A, Arev);
  }
  { // dense matrix
    Eigen::MatrixXd H(5, 3); // 5 * 3
    H << 1., 2., 3.,
      4., 5., 6.,
      7., 8., 9.,
      10., 11., 12.,
      13., 14., 15.;
    paracel::pkl_dat_sequential(H, "/var/tmp/test.pkl");
    Eigen::MatrixXd Hrev;
    paracel::unpkl_dat_sequential("/var/tmp/test.pkl", Hrev, 3);
    BOOST_CHECK_EQUAL_M(H, Hrev);
  }
  { // undirected_graph
    paracel::list_type<std::tuple<size_t, size_t> > edges;
    edges.emplace_back(std::make_tuple(0, 1));
    edges.emplace_back(std::make_tuple(0, 2));
    edges.emplace_back(std::make_tuple(0, 5));
    edges.emplace_back(std::make_tuple(0, 6));
    edges.emplace_back(std::make_tuple(3, 4));
    edges.emplace_back(std::make_tuple(3, 5));
    edges.emplace_back(std::make_tuple(4, 5));
    edges.emplace_back(std::make_tuple(4, 6));
    edges.emplace_back(std::make_tuple(9, 10));
    edges.emplace_back(std::make_tuple(9, 11));
    edges.emplace_back(std::make_tuple(9, 12));
    edges.emplace_back(std::make_tuple(11, 12));
    paracel::undirected_graph<size_t> grp(edges), grp2;
    paracel::pkl_dat_sequential(grp, "/var/tmp/test.pkl");
    paracel::unpkl_dat_sequential("/var/tmp/test.pkl", grp2);
    BOOST_CHECK_EQUAL(grp2.v(), 11);
    BOOST_CHECK_EQUAL(grp2.e(), 12);
    BOOST_CHECK_EQUAL(grp2.avg_degree(), 24. / 11.);
    BOOST_CHECK_EQUAL(grp2.max_degree(), 4); 
    BOOST_CHECK_EQUAL(grp2.selfloops(), 0); 
  }
  { // digraph
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
    paracel::digraph<size_t> grp(tpls), grp2;
    grp.add_edge(3, 4, 5.);
    paracel::pkl_dat_sequential(grp, "/var/tmp/test.pkl");
    paracel::unpkl_dat_sequential("/var/tmp/test.pkl", grp2);
    BOOST_CHECK_EQUAL(grp2.v(), 5);
    BOOST_CHECK_EQUAL(grp2.e(), 10);
    BOOST_CHECK_EQUAL(grp2.outdegree(0), 2);
    BOOST_CHECK_EQUAL(grp2.indegree(0), 3);
    BOOST_CHECK_EQUAL(grp2.avg_degree(), 2.);
    BOOST_CHECK_EQUAL(grp2.selfloops(), 3);
  }
  { // bigraph_continuous_test
    paracel::bigraph_continuous G, G2;
    G.add_edge(0, 1, 3.);
    G.add_edge(0, 2, 4.);
    G.add_edge(0, 4, 2.);
    G.add_edge(1, 3, 5.);
    G.add_edge(1, 4, 4.);
    G.add_edge(1, 5, 5.);
    G.add_edge(2, 4, 3.);
    G.add_edge(2, 5, 1.);
    G.add_edge(2, 6, 2.);
    G.add_edge(3, 3, 3.);
    paracel::pkl_dat_sequential(G, "/var/tmp/test.pkl");
    paracel::unpkl_dat_sequential("/var/tmp/test.pkl", G2);
    BOOST_CHECK_EQUAL(G2.v(), 4); 
    BOOST_CHECK_EQUAL(G2.e(), 10);
    BOOST_CHECK_EQUAL(G2.outdegree(0), 3); 
    BOOST_CHECK_EQUAL(G2.indegree(5), 2);
  }
  { // bigraph
    paracel::list_type<std::tuple<std::string, std::string, double> > tpls;
    tpls.emplace_back(std::make_tuple("a", "A", 3.));
    tpls.emplace_back(std::make_tuple("a", "B", 4.));
    tpls.emplace_back(std::make_tuple("a", "D", 2.));
    tpls.emplace_back(std::make_tuple("b", "C", 5.));
    tpls.emplace_back(std::make_tuple("b", "D", 4.));
    tpls.emplace_back(std::make_tuple("c", "D", 3.));
    tpls.emplace_back(std::make_tuple("b", "E", 5.));
    tpls.emplace_back(std::make_tuple("c", "E", 1.));
    tpls.emplace_back(std::make_tuple("c", "F", 2.));
    tpls.emplace_back(std::make_tuple("d", "C", 3.));
    paracel::bigraph<std::string> grp(tpls), grp2;
    grp.add_edge("c", "G", 3.6);

    paracel::pkl_dat_sequential(grp, "/var/tmp/test.pkl");
    paracel::unpkl_dat_sequential("/var/tmp/test.pkl", grp2);
    BOOST_CHECK_EQUAL(grp2.v(), 4);
    BOOST_CHECK_EQUAL(grp2.e(), 11);
    BOOST_CHECK_EQUAL(grp2.outdegree("a"), 3);
    BOOST_CHECK_EQUAL(grp2.indegree("E"), 2);
  }
}
