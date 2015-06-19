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
#include <google/gflags.h>
#include "sim_dense.hpp"
#include "utils.hpp"

using std::string;

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-servers.\n");
DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  paracel::json_parser jp(FLAGS_cfg_file);
  string input_a = jp.check_parse<string>("input_a");
  string input_b = jp.check_parse<string>("input_b");
  string output = jp.parse<string>("output");
  double simbar = jp.parse<double>("simbar");
  int ktop = jp.parse<int>("topk");

  paracel::alg::sim_dense solver(comm, FLAGS_server_info,
  				input_a, input_b, output,
				simbar, ktop);
  solver.solve();
  solver.dump_result();
  return 0;
}
