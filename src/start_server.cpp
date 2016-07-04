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

#include <gflags/gflags.h>

#include "server.hpp"

DEFINE_string(start_host, "beater7", "host name of start node\n");
DEFINE_string(init_port, "7773", "init port");

int main(int argc, char *argv[])
{
  google::SetUsageMessage("[options]\n\
  			--start_host\tdefault: balin\n\
			--init_port\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  paracel::init_thrds(FLAGS_start_host, FLAGS_init_port); // join inside
  return 0;
}
