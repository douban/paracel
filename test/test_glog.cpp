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

#include <glog/logging.h>
#include <mpi.h>
#include "utils/comm.hpp"

int main(int argc, char* argv[]) 
{  
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  
  google::InitGoogleLogging(argv[0]);
  int num_cookies = 7;
  LOG(INFO) << "Found " << num_cookies << " cookies";
  LOG_IF(INFO, comm.get_rank() == 0) << "Got lots of cookies";
  
  DLOG(INFO) << "Found cookies";
  DLOG_IF(INFO, comm.get_rank() == 0) << "Got lots of cookies";

  VLOG_IF(1, comm.get_rank() == 0)
      << "I'm printed when size is more than 1024 and when you run the "
      "program with --v=1 or more";
  return 0;
}

