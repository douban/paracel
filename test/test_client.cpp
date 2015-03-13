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

//#define BOOST_TEST_DYN_LINK
//#define BOOST_TEST_MODULE CLIENT_TEST 

#include <iostream>
#include <string>
#include <mpi.h>
//#include <boost/test/unit_test.hpp>
#include "ps.hpp"
#include "utils.hpp"
#include "client.hpp"
using std::cout;
using std::endl;

//BOOST_AUTO_TEST_CASE (client_test) {
int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);
  
  /*
  auto ports_tmp = paracel::get_hostnames_string(1, "6378");
  char hostname[1024];
  gethostname(hostname, sizeof(hostname));
  
  std::string server_name = hostname;
  std::string ports(ports_tmp.begin() + server_name.size() + 1,
                    ports_tmp.end());
  std::cout << ports << std::endl;
  */

  std::string server_name = "beater7";
  std::string ports = "48121,53146,58489,12740,27132";

  paracel::kvclt kvc(server_name, ports);

  {
    std::string prefix = "test_key_";
    kvc.push(prefix, 7.);
    MPI_Barrier(MPI_COMM_WORLD);
    kvc.pull<double>(prefix);
  }
  MPI_Finalize();
}
