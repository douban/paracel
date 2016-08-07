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

#include <stdexcept>
#include <string>
#include <mpi.h>
#include <gflags/gflags.h>
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
  std::string user_fac_input, item_fac_input, output, handle_fn;
  int level, tree_start_indx, tree_end_indx;
  try {
    user_fac_input = pt.check_parse<std::string>("user_fac_input");
    item_fac_input = pt.check_parse<std::string>("item_fac_input");
    output = pt.parse<std::string>("output");
    handle_fn = pt.check_parse<std::string>("handle_file");
    level = pt.parse<int>("level");
    tree_start_indx = pt.parse<int>("tree_start_index");
    tree_end_indx = pt.parse<int>("tree_end_index");
  } catch (const std::invalid_argument & e) {
    std::cerr << e.what();
    return 1;
  }
  paracel::alg::decision_tree_builder_factor solver(comm,
                                                    FLAGS_server_info,
                                                    user_fac_input,
                                                    item_fac_input,
                                                    paracel::add_folder_suffix_with_date(output),
                                                    handle_fn,
                                                    level,
                                                    tree_start_indx,
                                                    tree_end_indx);
  solver.solve();
  solver.dump();
  return 0;
}
