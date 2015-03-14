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

#include "wc.hpp"
#include "utils.hpp"

DEFINE_string(server_info,
              "host1:7777PARACELhost2:8888",
              "hosts name string of paracel-servers.\n");

DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  
  paracel::json_parser pt(FLAGS_cfg_file);
  std::string input = pt.parse<std::string>("input");
  std::string output = pt.parse<std::string>("output");
  int topk = pt.parse<int>("topk");
  std::string handle_fn = pt.parse<std::string>("handle_file");
  std::string update_fcn = pt.parse<std::string>("update_function");
  std::string filter_fcn = pt.parse<std::string>("filter_function");
  
  paracel::alg::word_count wc_solver(comm,
                                     FLAGS_server_info,
                                     input,
                                     output,
                                     topk,
                                     handle_fn,
                                     update_fcn,
                                     filter_fcn);
  wc_solver.solve();
  wc_solver.print();
  return 0;
}
