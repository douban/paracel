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

#include <google/gflags.h>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"
#include "paracel_types.hpp"

using std::string;
using std::vector;
using paracel::paralg;
using paracel::Comm;
using paracel::str_split;

namespace paracel {
namespace tool {

using node_t = paracel::default_id_type;

class svd {

public:  
  svd(Comm comm,
  	string _input, 
  	string _output,
    int _k) : input(_input), k(_k) {
    pt = new paralg(comm, _output);
    rank = pt->get_worker_id();
    np = pt->get_worker_size();
  }
  
  ~svd() {
    delete pt;
  } 

  double loss(const Eigen::MatrixXd & A,
              const Eigen::MatrixXd & result) {
    return sqrt((A - result).squaredNorm() / (A.rows() * A.cols()));
  }

  Eigen::MatrixXd mf(const Eigen::MatrixXd & AA,
                     Eigen::MatrixXd & WW,
                     Eigen::MatrixXd & HH) {
    HH = Eigen::MatrixXd::Random(AA.cols(), k);
    for(int iter = 0; iter < 10; ++iter) {
      auto tmp1 = HH.transpose() * HH;
      WW = AA * HH * tmp1.inverse();
      auto tmp2 = WW.transpose() * WW;
      HH = AA.transpose() * WW * tmp2.inverse();
    }
    return WW * HH.transpose();
  }

  void qr_iteration(const Eigen::MatrixXd & M,
                    Eigen::MatrixXd & q,
                    Eigen::MatrixXd & r) {
    Eigen::MatrixXd MtM = M.transpose() * M;
    Eigen::LLT<Eigen::MatrixXd> lltOfA(MtM);
    Eigen::MatrixXd L = lltOfA.matrixL();
    r = -L.transpose();
    q = -M * L.transpose().inverse();
  }

  void subsvd(const Eigen::MatrixXd & r_W,
              const Eigen::MatrixXd & r_H,
              Eigen::MatrixXd & S,
              Eigen::MatrixXd & r_U,
              Eigen::MatrixXd & r_V) {
    //std::cout << "r_W * r_H.transpose() " << r_W * r_H.transpose() << std::endl;
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(r_W * r_H.transpose(),
                                          Eigen::ComputeThinU | Eigen::ComputeThinV);
    S = svd.singularValues(); // rank * 1
    r_U = svd.matrixU();
    r_V = svd.matrixV();
  }

  void solve() {
    auto parser_lambda = [] (const string & line) {
      return str_split(line, ',');
    };
    auto f_parser = paracel::gen_parser(parser_lambda);
    pt->paracel_load_as_matrix(A, row_map, col_map, input, f_parser);

    // first stage
    Eigen::MatrixXd W, H;
    auto approximate_A = mf(A, W, H);
    //std::cout << "mf loss: " << loss(A, approximate_A);

    // second stage
    Eigen::MatrixXd q_W, q_H, r_W, r_H;
    qr_iteration(W, q_W, r_W);
    qr_iteration(H, q_H, r_H);
    //std::cout << q_W * r_W * r_H.transpose() * q_H.transpose() << std::endl;
    Eigen::MatrixXd r_U, r_V;
    subsvd(r_W, r_H, sigma, r_U, r_V);
    U = q_W * r_U;
    V = q_H * r_V;
    assert(U.cols() == k);
    assert(V.cols() == k);
  }

  void dump_result() {
    // dump sigma
    std::vector<double> dump_sigma;
    for(int i = 0; i < k; ++i) {
      dump_sigma.push_back(sigma.row(i)[0]);
    }
    pt->paracel_dump_vector(dump_sigma, "SIGMA_");

    // dump U
    std::unordered_map<node_t, std::vector<double> > dump_u;
    for(int i = 0; i < U.rows(); ++i) {
      for(int j = 0; j < k; ++j) {
        dump_u[row_map[i]].push_back(U.row(i)[j]);
      }
    }
    pt->paracel_dump_dict(dump_u, "U_");

    std::unordered_map<node_t, std::vector<double> > dump_v;
    // dump V
    for(int i = 0; i < V.rows(); ++i) {
      for(int j = 0; j < k; ++j) {
        dump_v[col_map[i]].push_back(V.row(i)[j]);
      }
    }
    pt->paracel_dump_dict(dump_v, "V_");
  }

 private:
  string input;
  int k;
  paralg *pt;
  int rank, np;
  Eigen::SparseMatrix<double, Eigen::RowMajor> A; // M * N
  std::unordered_map<paracel::default_id_type, node_t> row_map, col_map;
  Eigen::MatrixXd sigma, U, V;
}; // class svd

} // namespace tool 
} // namespace paracel

DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  
  google::SetUsageMessage("[options]\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);

  paracel::json_parser pt(FLAGS_cfg_file);
  std::string input = pt.check_parse<std::string>("input");
  std::string output = pt.parse<std::string>("output");
  int k = pt.parse<int>("k");
  paracel::tool::svd svd_solver(comm, input, output, k);
  svd_solver.solve();
  svd_solver.dump_result();
  return 0;
}
