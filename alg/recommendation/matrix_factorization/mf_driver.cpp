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

#include <string>
#include <iostream>

#include <mpi.h>
#include <google/gflags.h>

#include "mf.hpp"
#include "utils.hpp"

DEFINE_string(server_info,
              "host1:7777PARACELhost2:8888",
              "hosts name string of paracel-servers.\n");

DEFINE_string(cfg_file,
              "",
              "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  
  paracel::json_parser pt(FLAGS_cfg_file);
  std::string input = pt.parse<std::string>("input");
  std::string pred_input = pt.parse<std::string>("predict_input");
  std::string output = pt.parse<std::string>("output");
  std::string update_fn = pt.parse<std::string>("update_file");
  std::vector<std::string> update_funcs = pt.parse_v<std::string>("update_functions");
  std::string filter_fn = pt.parse<std::string>("filter_file");
  std::vector<std::string> filter_funcs = pt.parse_v<std::string>("filter_functions");
  int k = pt.parse<int>("k");
  int rounds = pt.parse<int>("rounds");
  double alpha = pt.parse<double>("alpha");
  double beta = pt.parse<double>("beta");
  bool debug = pt.parse<bool>("debug");
  bool ssp_switch = pt.parse<bool>("ssp_switch");
  int limit_s = pt.parse<int>("limit_s");
  
  paracel::alg::matrix_factorization mf_solver(comm,
                                               FLAGS_server_info,
                                               input,
                                               pred_input,
                                               output,
                                               update_fn,
                                               update_funcs,
                                               filter_fn,
                                               filter_funcs,
                                               k,
                                               rounds,
                                               alpha,
                                               beta,
                                               debug,
                                               ssp_switch,
                                               limit_s);
  mf_solver.solve();
  std::cout << "rmse: " << mf_solver.cal_rmse() << std::endl;
  mf_solver.predict();
  mf_solver.dump_result();
  return 0;
}
