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

// unit test for comm.hpp(worker number = 3)

#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

#include "utils/comm.hpp"

using namespace std;

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  auto rk = comm.get_rank();
  //auto sz = comm.get_size();

  {
    double val = static_cast<double>(rk);
    auto reduce_lambda = [] (const double & recv, double & send) {
      send += recv;
    };
    val = comm.treereduce(val, reduce_lambda, 2);
    if(rk == 2) {
      std::cout << val << std::endl;
    }
  }
  
  {
    unordered_map<string, double> d;
    string key = "rk_key" + std::to_string(rk);
    d[key] = static_cast<double>(rk);
    auto reduce_lambda = [] (const unordered_map<string, double> & recvbuf,
                              std::unordered_map<string, double> & sendbuf) {
      for(auto & kv : recvbuf) {
        if(sendbuf.count(kv.first) == 0) {
          sendbuf[kv.first] = kv.second;
        }
      }
    };
    d = comm.treereduce(d, reduce_lambda, 1);
    if(rk == 1) {
      for(auto & kv : d) {
        std::cout << "key: " << kv.first << " val: " << kv.second << std::endl;
      }
    }
  }
  return 0;
}
