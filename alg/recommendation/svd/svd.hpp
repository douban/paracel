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
 * Authors: 
 * Hong Wu <xunzhangthu@gmail.com>
 * Changsheng Jiang <jiangzuoyan@gmail.com>
 *
 */

/**
 * A novel two-stage(linear scalable) approach to do 
 * Singular Value Decomposition which can be applied to solve symmetric eigen problems.
 * Suppose input data is being fillinged in advance
 *
 */

#ifndef FILE_e0a89d15_2922_9751_4d01_f0efe89a703b_HPP
#define FILE_e0a89d15_2922_9751_4d01_f0efe89a703b_HPP

#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/QR>
#include <eigen3/Eigen/Cholesky>
#include <eigen3/Eigen/SVD>

#include "ps.hpp"
#include "utils.hpp"
#include "load.hpp"
#include "paracel_types.hpp"

using std::string;
using std::vector;
using std::unordered_map;

namespace paracel {
namespace alg {

using node_t = paracel::default_id_type;
using eigen_triple = Eigen::Triplet<double>;

class svd : public paracel::paralg {
  
 public:
  svd(paracel::Comm comm,
      string hosts_dct_str,
      string _input,
      string _output,
      string update_fn,
      string update_fcn,
      int _k) : paracel::paralg(hosts_dct_str,
                                comm,
                                _output),
                output(_output),
                update_file(update_fn),
                update_function(update_fcn),
                k(_k) {
    rank = get_worker_id();
    np = get_worker_size();
  }
  
  /*
  void init() {
    auto parser_lambda = [] (const string & line) {
      return paracel::str_split(line, ',');
    };
    auto f_parser = paracel::gen_parser(parser_lambda);
    paracel_load_as_matrix(blk_A,
                           row_map,
                           col_map,
                           input,
                           f_parser,
                           "fmap");

    // init exchange matrix P
    std::unorderd_map<node_t, size_t> reverse_col_map;
    for(auto & kv : col_map) {
      reverse_col_map[kv.second] = kv.first;
    }
    P.resize(0);
    for(auto i = 0; i < row_map.size(); ++i) {
      node_t rid = row_map[i];
      for(auto & kv : col_map) {
        node_t cid = kv.second;
        if(cid == rid) {
          P.push_back(reverse_col_map[cid]);
        }
      }
    }
    reverse_col_map.clear();
    paracel_write("exchange_P_" + std::to_string(rank), P);
    P.resize(0);
    paracel_sync();

    for(size_t k = 0; k < np; ++k) {
      auto tmp_P = paracel_read<vector<size_t> >("exchange_P_" + std::to_string(k));
      P.insert(P.end(), tmp_P.begin(), tmp_P.end());
    }
    vector<Eigen::Triplet<double> > nonzero_tpls;
    for(size_t i = 0; i < P.size(); ++i) {
      nonzero_tpls.push_back(Eigen::Triplet<double>(i, P[i], 1.));
    }
    exchange_P.resize(P.size(), P.size());
    exchange_P.setFromTriplets(nonzero_tpls.begin(), nonzero_tpls.end());
    nonzero_tpls.resize(0);

    C = blk_A.rows();
    N = blk_A.cols();
    K = k + over_sampling;
    paracel_write("global_C_indx_" + std::to_string(rank), C);
    global_indx.resize(0);
    global_indx.push_back(0);
    paracel_sync();
    size_t accum = 0;
    for(size_t k = 0; k < np; ++k) {
      size_t indx = paracel_read<size_t>("global_C_indx" + std::to_string(k));
      accum += indx;
      global_indx.push_back(accum);
    }
  }
  */

  // parallel compute then return H' * H, where H is a n by k dense matrix
  // H_blk: columns block of H'(rows block of H)
  Eigen::MatrixXd 
  parallel_mm_kxn_dense_by_nxk_dense(const Eigen::MatrixXd & H_blk,
                                     const std::string & keyname) {
    Eigen::MatrixXd local_result = H_blk.transpose() * H_blk;
    std::vector<double> local_result_vec = paracel::mat2vec(local_result.transpose()); 
    paracel_bupdate(keyname,
                    local_result_vec,
                    update_file,
                    update_function);
    paracel_sync();
    std::vector<double> local_result_vec_new = paracel_read<std::vector<double> >(keyname);
    return paracel::vec2mat(local_result_vec_new, K);
  }

  // parallel compute mA * mH, where mA is a m by n sparse matrix 
  // and mH is a n by k dense matrix
  // return rows block of mA * mH
  // A_blk: columns block of mA, H_blk: rows block of mH
  Eigen::MatrixXd 
  parallel_mm_nxn_sparse_by_nxk_dense(const Eigen::SparseMatrix<double, Eigen::RowMajor> & A_blk,
                                      const Eigen::MatrixXd & H_blk,
                                      const std::string & keyname) {
    int blk_size = A_blk.rows() / np; 
    for(size_t k = 0; k < np; ++k) {
      int cols = A_blk.cols();
      int rows = blk_size;
      if(k == (np - 1)) {
        rows += A_blk.rows() % np;
      }
      Eigen::MatrixXd part_A_blk = Eigen::MatrixXd(A_blk).block(k * blk_size, 0, rows, cols);
      Eigen::MatrixXd local_result = part_A_blk * H_blk;
      std::vector<double> local_result_vec = paracel::mat2vec(local_result.transpose());
      paracel_bupdate(keyname + std::to_string(k),
                      local_result_vec,
                      update_file,
                      update_function);
    } // end for
    paracel_sync();
    auto data = paracel_read<std::vector<double> >(keyname + std::to_string(rank));
    int rows = blk_size;
    if(rank == np - 1) {
      rows += A_blk.rows() % np;
    }
    return paracel::vec2mat(data, rows);
  }

  void matrix_factorization(const Eigen::SparseMatrix<double, Eigen::RowMajor> & A_blk,
                            const Eigen::SparseMatrix<double, Eigen::RowMajor> & At_blk,
                            Eigen::MatrixXd & W_blk,
                            Eigen::MatrixXd & H_blk) {
    //srand((unsigned)time(NULL));
    size_t accum_rows = 0;
    size_t total_rows = 0;
    for(size_t k = 0; k < np; ++k) {
      size_t _rows = paracel_read<size_t>("global_C_indx_" + std::to_string(k));
      if(k < get_worker_id()) {
        accum_rows += _rows;
      }
      total_rows += _rows;
    }
    Eigen::MatrixXd H_global = Eigen::MatrixXd::Random(total_rows, K);
    H_blk = H_global.block(accum_rows, 0, C, K);
    if(rank == 0) std::cout << H_blk << std::endl;
    paracel_sync();
    for(int iter = 0; iter < 20; ++iter) {
      // W = A * H * inv(H' * H)
      Eigen::MatrixXd HtH = parallel_mm_kxn_dense_by_nxk_dense(H_blk, "HtH");
      if(rank == 0) std::cout << HtH << std::endl;
      paracel_sync();
      Eigen::MatrixXd AH_blk = parallel_mm_nxn_sparse_by_nxk_dense(At_blk, H_blk, "AH_");
      W_blk = AH_blk * HtH.inverse();
      if(rank == 0) std::cout << W_blk << std::endl;
      paracel_sync();
      // H = A' * W * inv(W' * W)
      Eigen::MatrixXd WtW = parallel_mm_kxn_dense_by_nxk_dense(W_blk, "WtW");
      paracel_sync();
      Eigen::MatrixXd AtW_blk = parallel_mm_nxn_sparse_by_nxk_dense(A_blk.transpose(), W_blk, "AtW_");
      H_blk = AtW_blk * WtW.inverse();
    }
    paracel_sync();
  }

  // QR of M (n by k here) is equivalent to the Cholesky decomposition of M'M = L'L
  // where q = -M * L^ (-1), r = -L
  void qr_iteration(const Eigen::MatrixXd & M_blk,
                    const std::string & keyname,
                    Eigen::MatrixXd & q_blk,
                    Eigen::MatrixXd & r) {
    Eigen::MatrixXd MtM = parallel_mm_kxn_dense_by_nxk_dense(M_blk, keyname);
    paracel_sync();
    Eigen::LLT<Eigen::MatrixXd> lltOfA(MtM);
    Eigen::MatrixXd L = lltOfA.matrixL();
    r = -L.transpose();
    q_blk = -M_blk * L.transpose().inverse(); // here L.inverse is equal to L^(-1)
  }

  virtual void solve() {
    //init();
    C = 2;
    N = 4;
    K = 3;
    paracel_write("global_C_indx_" + std::to_string(rank), C);

    if(rank == 0) {
      std::vector<eigen_triple> tpls;
      tpls.push_back(eigen_triple(0, 1, 0.9));
      tpls.push_back(eigen_triple(0, 2, 0.8));
      tpls.push_back(eigen_triple(1, 0, 0.8));
      tpls.push_back(eigen_triple(1, 1, 0.2));
      tpls.push_back(eigen_triple(1, 3, 0.1));
      blk_A.resize(2, 4);
      blk_A.setFromTriplets(tpls.begin(), tpls.end());
      std::vector<eigen_triple> tpls2;
      tpls.push_back(eigen_triple(0, 1, 0.9));
      tpls.push_back(eigen_triple(1, 0, 0.8));
      tpls.push_back(eigen_triple(1, 1, 0.2));
      tpls.push_back(eigen_triple(2, 0, 0.9));
      tpls.push_back(eigen_triple(3, 1, 0.3));
      blk_A_T.resize(4, 2);
      blk_A_T.setFromTriplets(tpls2.begin(), tpls2.end());
    }
    if(rank == 1) {
      std::vector<eigen_triple> tpls;
      tpls.push_back(eigen_triple(0, 0, 0.9));
      tpls.push_back(eigen_triple(0, 2, 0.2));
      tpls.push_back(eigen_triple(0, 3, 0.3));
      tpls.push_back(eigen_triple(1, 1, 0.3));
      tpls.push_back(eigen_triple(1, 2, 0.1));
      blk_A.resize(2, 4);
      blk_A.setFromTriplets(tpls.begin(), tpls.end());
      std::vector<eigen_triple> tpls2;
      tpls.push_back(eigen_triple(0, 0, 0.8));
      tpls.push_back(eigen_triple(1, 1, 0.1));
      tpls.push_back(eigen_triple(2, 0, 0.2));
      tpls.push_back(eigen_triple(2, 1, 0.3));
      tpls.push_back(eigen_triple(3, 0, 0.1));
      blk_A_T.resize(4, 2);
      blk_A_T.setFromTriplets(tpls2.begin(), tpls2.end());
    }
    paracel_sync();

    // first stage: do matrix factorization
    Eigen::MatrixXd blk_W, blk_H; // size: C * K | C * K
    matrix_factorization(blk_A, blk_A_T, blk_W, blk_H);
    if(rank == 0) std::cout << blk_W << std::endl;
    std::cout << "done" << std::endl;
    if(rank == 0) std::cout << blk_H << std::endl;
    paracel_sync();

    // second stage: qr decomposition then do sub svd
    Eigen::MatrixXd q_W_blk, r_W; // size: N * K | K * K
    qr_iteration(blk_W, "WtW_qr", q_W_blk, r_W);
    
    Eigen::MatrixXd q_H_blk, r_H; // size: N * K | K * K
    qr_iteration(blk_H, "HtH_qr", q_H_blk, r_H);
    paracel_sync();

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(r_W * r_H.transpose(),
                                          Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::MatrixXd sigma = svd.singularValues(); // rank * 1
    Eigen::MatrixXd U_r = svd.matrixU();
    Eigen::MatrixXd V_r = svd.matrixV();
    paracel_sync();

    // group back
    Eigen::MatrixXd U_blk = q_W_blk * U_r;
    Eigen::MatrixXd V_blk = q_H_blk * V_r;
    paracel_sync();
  }
  
 private:
  string input, output;
  string update_file, update_function;
  int k;
  int over_sampling = 10;
  
  Eigen::SparseMatrix<double, Eigen::RowMajor> blk_A; // size: C * N
  Eigen::SparseMatrix<double, Eigen::RowMajor> blk_A_T; // size: N * C
  Eigen::SparseMatrix<double, Eigen::RowMajor> exchange_P;
  std::unordered_map<size_t, node_t> row_map, col_map;

  std::vector<size_t> P;
  size_t rank, np;
  size_t N, C, K;

}; // class svd

} // namespace alg
} // namespace paracel

#endif
