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

#include <ctime>
#include <string>
#include <mpi.h>
#include <google/gflags.h>
#include "decision_tree_builder.hpp"
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
  std::string user_fac_input = pt.parse<std::string>("user_fac_input");
  std::string item_fac_input = pt.parse<std::string>("item_fac_input");
  std::string output = pt.parse<std::string>("output");
  time_t t = time(0); // get time now
  struct tm *now = localtime(&t);
  std::string output_suffix = std::to_string(now->tm_year + 1900) + std::to_string(now->tm_mon + 1) + std::to_string(now->tm_mday);
  std::string handle_fn = pt.parse<std::string>("handle_file");
  int level = pt.parse<int>("level");
  int tree_start_indx = pt.parse<int>("tree_start_index");
  int tree_end_indx = pt.parse<int>("tree_end_index");

  paracel::alg::decision_tree_builder_factor solver(comm,
                                                    FLAGS_server_info,
                                                    user_fac_input,
                                                    item_fac_input,
                                                    output + output_suffix,
                                                    handle_fn,
                                                    level,
                                                    tree_start_indx,
                                                    tree_end_indx);
  solver.solve();
  solver.dump();
  return 0;
}
