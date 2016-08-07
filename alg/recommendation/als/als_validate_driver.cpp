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

#include "als.hpp"
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
  std::string rating_input, factor_input, validate_input, output, pattern;
  std::vector<double> lambdas;
  try {
    rating_input = pt.check_parse<std::string>("rating_input");
    factor_input = pt.check_parse<std::string>("factor_input");
    validate_input = pt.check_parse<std::string>("validate_input");
    output = pt.parse<std::string>("output");
    pattern = pt.parse<std::string>("pattern");
    lambdas = pt.parse_v<double>("lambdas");
  } catch (const std::invalid_argument & e) {
    std::cerr << e.what();
    return 1;
  }
  // model selection
  {
    std::vector<double> validate_errs, train_errs;
    paracel::alg::alternating_least_square_validate H_solver(comm, FLAGS_server_info,
                                                             rating_input,
                                                             factor_input,
                                                             validate_input,
                                                             output,
                                                             pattern);

    H_solver.init();
    try {
      for(auto & lambda : lambdas) {
        H_solver.learning(lambda);
        train_errs.push_back(H_solver.cal_rmse());
        validate_errs.push_back(H_solver.validate());
      }
    } catch(const std::runtime_error & e) {
      std::cerr << e.what();
      return 1;
    }
    if(comm.get_rank() == 0) {
      for(size_t i = 0; i < train_errs.size(); ++i) {
        std::cout << "###################################" << std::endl;
        std::cout << "lambda: " << lambdas[i] << std::endl;
        std::cout << "train error: " << train_errs[i] << std::endl;
        std::cout << "validate error: " << validate_errs[i] << std::endl;
        std::cout << "###################################" << std::endl;
      }
    }
  }

  return 0;
}
