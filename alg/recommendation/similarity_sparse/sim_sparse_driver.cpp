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
#include <stdexcept>
#include <gflags/gflags.h>
#include "sim_sparse_row.hpp"
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
  paracel::json_parser pt(FLAGS_cfg_file);
  string input, output, fmt, sim_method;
  int ktop, rbar, cbar, rubar, cubar;
  double sim_bar, sim_ubar, weight_bar, intersect_bar;
  try {
    input = pt.check_parse<string>("input");
    output = pt.parse<string>("output");
    fmt = pt.parse<string>("format");
    ktop = pt.parse<int>("topk");
    rbar = pt.parse<int>("rbar");
    cbar = pt.parse<int>("cbar");
    rubar = pt.parse<int>("rubar");
    cubar = pt.parse<int>("cubar");
    sim_bar = pt.parse<double>("sim_bar");
    sim_ubar = pt.parse<double>("sim_ubar");
    weight_bar = pt.parse<double>("weight_bar");
    intersect_bar = pt.parse<double>("intersect_bar");
    sim_method = pt.parse<string>("sim_method");
  } catch (const std::invalid_argument & e) {
    std::cerr << e.what();
    return 1;
  }
  paracel::alg::sim_sparse_row solver(comm, FLAGS_server_info,
                                      input, output,
                                      fmt,
                                      ktop,
                                      rbar, cbar,
                                      rubar, cubar,
                                      sim_bar, sim_ubar,
                                      weight_bar,
                                      intersect_bar,
                                      sim_method);
  solver.solve();
  solver.dump_result();
  return 0;
}
