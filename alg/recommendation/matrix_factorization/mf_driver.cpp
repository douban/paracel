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
#include <stdexcept>

#include <mpi.h>
#include <gflags/gflags.h>

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
  std::string input, pred_input, output, update_fn, filter_fn;
  std::vector<std::string> update_funcs, filter_funcs;
  int k, rounds, limit_s;
  double alpha, beta;
  bool debug, ssp_switch;
  try {
    input = pt.check_parse<std::string>("input");
    pred_input = pt.check_parse<std::string>("predict_input");
    output = pt.parse<std::string>("output");
    update_fn = pt.check_parse<std::string>("update_file");
    update_funcs = pt.parse_v<std::string>("update_functions");
    filter_fn = pt.check_parse<std::string>("filter_file");
    filter_funcs = pt.parse_v<std::string>("filter_functions");
    k = pt.parse<int>("k");
    rounds = pt.parse<int>("rounds");
    alpha = pt.parse<double>("alpha");
    beta = pt.parse<double>("beta");
    debug = pt.parse<bool>("debug");
    ssp_switch = pt.parse<bool>("ssp_switch");
    limit_s = pt.parse<int>("limit_s");
  } catch (const std::invalid_argument & e) {
    std::cerr << e.what();
    return 1;
  }
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
