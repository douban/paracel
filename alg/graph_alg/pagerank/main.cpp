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
#include <google/gflags.h>

#include "pr.hpp"
#include "utils.hpp"

using std::string;
using std::cout;
using std::endl;

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-servers.\n");
DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  
  paracel::json_parser jp(FLAGS_cfg_file);
  string input = jp.parse<string>("input");
  string output = jp.parse<string>("output");
  int rounds = jp.parse<int>("rounds");
  double df = jp.parse<double>("damping_factor");
  string handle_fn = jp.parse<string>("handle_file");
  string update_fcn = jp.parse<string>("update_function");
  string filter_fcn = jp.parse<string>("filter_function");
  
  paracel::alg::pagerank pr_solver(comm,
                                   FLAGS_server_info,
                                   input,
                                   output,
                                   handle_fn,
                                   update_fcn,
                                   filter_fcn,
                                   rounds,
                                   df);
  pr_solver.solve();
  pr_solver.dump_result();
  return 0;
}
