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
#include <exception>

#include <mpi.h>
#include <gflags/gflags.h>

#include "max_vertex_value.hpp"
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
  std::string input, output, update_file, update_func;
  try {
    input = pt.check_parse<std::string>("input");
    output = pt.parse<std::string>("output");
    update_file = pt.check_parse<std::string>("update_file");
    update_func = pt.parse<std::string>("update_func");
  } catch (const std::invalid_argument & e) {
    std::cerr << e.what();
    return 1;
  }
  paracel::alg::max_vertex_value mvv_solver(comm,
                                            FLAGS_server_info,
                                            input,
                                            output,
                                            update_file,
                                            update_func);
  mvv_solver.solve();
  mvv_solver.dump_result();
  return 0;
}
