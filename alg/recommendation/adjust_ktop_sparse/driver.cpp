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

#include "adjust_ktop_sparse.hpp"
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
  std::string rating_input = pt.check_parse<std::string>("rating_input");
  std::string sim_input = pt.check_parse<std::string>("sim_input");
  std::string output = pt.parse<std::string>("output");
  paracel::alg::adjust_ktop_s solver(comm, 
                                     FLAGS_server_info,
                                     rating_input,
                                     sim_input,
                                     output);
  solver.solve();
  //solver.dump_result();
  return 0;
}
