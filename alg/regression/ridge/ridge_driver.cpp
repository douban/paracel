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

#include "ridge.hpp"
#include "utils.hpp"

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-servers.\n");
DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);

  paracel::json_parser pt(FLAGS_cfg_file);
  std::string training_input, test_input, predict_input, output, update_file, update_func;
  int rounds;
  double alpha, beta;
  try {
    training_input = pt.check_parse<std::string>("training_input");
    test_input = pt.check_parse<std::string>("test_input");
    predict_input = pt.check_parse<std::string>("predict_input");
    output = pt.parse<std::string>("output");
    update_file = pt.check_parse<std::string>("update_file");
    update_func = pt.parse<std::string>("update_func");
    rounds = pt.parse<int>("rounds");
    alpha = pt.parse<double>("alpha");
    beta = pt.parse<double>("beta");
  } catch (const std::invalid_argument & e) {
    std::cerr << e.what();
    return 1;
  }
  paracel::alg::ridge_regression solver(comm,
                                        FLAGS_server_info,
                                        training_input,
                                        output,
                                        update_file,
                                        update_func,
                                        rounds,
                                        alpha,
                                        beta);
  solver.solve();
  std::cout << "final loss: " << solver.calc_loss() << std::endl;
  solver.test(test_input);
  solver.predict(predict_input);
  solver.dump_result();
  return 0;
}
