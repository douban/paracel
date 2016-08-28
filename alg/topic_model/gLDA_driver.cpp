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
 * Authors: Ping Qin <qinping@douban.com>
 *
 */

#include <string>
#include <iostream>
#include <stdexcept>

#include <mpi.h>
#include <gflags/gflags.h>

#include "gLDA.hpp"
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
  std::string input, output, handle_fn;
  double alpha, beta;
  int k_topics, iters, top_words;
  bool debug;
  try {
    input = pt.check_parse<std::string>("input");
    output = pt.parse<std::string>("output");
    alpha = pt.parse<double>("alpha");
    beta = pt.parse<double>("beta");
    k_topics = pt.parse<int>("k_topics");
    iters = pt.parse<int>("iters");
    top_words = pt.parse<int>("top_words");
    handle_fn = pt.check_parse<std::string>("handle_file");
    debug = pt.parse<bool>("debug");
  } catch (const std::invalid_argument & e) {
    std::cerr << e.what();
    return 1;
  }
  paracel::alg::LDAmodel solver(comm,
                                FLAGS_server_info,
                                input,
                                output,
                                alpha,
                                beta,
                                k_topics,
                                iters,
                                top_words,
                                handle_fn,
                                debug);
  solver.train();
  solver.save();
  return 0;
}
