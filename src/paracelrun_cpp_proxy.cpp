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

#include "utils.hpp"

DEFINE_string(init_port, "7773", "init port");
DEFINE_int64(nsrv, 1, "number of parameter servers");

int main(int argc, char *argv[])
{
  google::SetUsageMessage("[options]\n\
  			--nsrv\n\
  			--init_port\n") ;
  google::ParseCommandLineFlags(&argc, &argv, true);
  std::string serverinfo = paracel::get_hostnames_string(FLAGS_nsrv, FLAGS_init_port);
  std::cout << serverinfo;
  return 0;
}
