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

// unit test for comm.hpp(worker number = 2)

#include <iostream>
#include <vector>
#include <tuple>
#include <string>
#include <set>
#include <unordered_map>
#include <mpi.h>

#include "utils/comm.hpp"

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD); 
  int rk = comm.get_rank();
  int sz = comm.get_size();
  comm.get_rank();

  { // builtin send + recv
    if(rk == 0) {
      int a = 7; 
      comm.send(a, 1, 2014);
    } else if(rk == 1){
      int b = 0;
      comm.recv(b, 0, 2014);
      std::cout << b << std::endl;
    }
  }
  
  { // isend + recv
    if(rk == 0) {
      int a = 7; 
      paracel::vrequest req;
      req = comm.isend(a, 1, 2014);
    } else if(rk == 1){
      int b = 0;
      comm.recv(b, 0, 2014);
      std::cout << b << std::endl;
    }
  }

  { // container send + recv
    if(rk == 0) {
      std::vector<int> aa {77, 88};
      comm.send(aa, 1, 2014);
    } else if(rk == 1) {
      std::vector<int> bb;
      comm.recv(bb, 0, 2014);
      for(auto & item : bb)
        std::cout << item << std::endl;
    }
  }

  { // container isend + recv
    if(rk == 0) {
      std::vector<int> aa {77, 88};
      paracel::vrequest req;
      req = comm.isend(aa, 1, 2014);
    } else if(rk == 1) {
      std::vector<int> bb;
      comm.recv(bb, 0, 2014);
      for(auto & item : bb)
        std::cout << item << std::endl;
    }
  }

  { // paracel triple send + recv
    if(rk == 0) {
      std::tuple<std::string, std::string, double> aa;
      std::get<0>(aa) = "abc";
      std::get<1>(aa) = "def";
      std::get<2>(aa) = 3.14;
      paracel::vrequest req;
      req = comm.isend(aa, 1, 2014);
    } else if(rk == 1) {
      std::tuple<std::string, std::string, double> bb;
      comm.recv(bb, 0, 2014);
      std::cout << "deniug" << std::get<0>(bb) << "--" << std::get<1>(bb) << "--" << std::get<2>(bb) << std::endl;
    }
  }

  { // paracel list of triple send + recv
    if(rk == 0) {
      std::vector<std::tuple<std::string, std::string, double> > aa;
      std::tuple<std::string, std::string, double> tmp1;
      std::get<0>(tmp1) = "abc";
      std::get<1>(tmp1) = "def";
      std::get<2>(tmp1) = 4.15;
      aa.push_back(tmp1);
      std::tuple<std::string, std::string, double> tmp2;
      std::get<0>(tmp2) = "cba";
      std::get<1>(tmp2) = "fed";
      std::get<2>(tmp2) = 5.16;
      aa.push_back(tmp2);
      paracel::vrequest req;
      req = comm.isend(aa, 1, 2014);
    } else if(rk == 1) {
      std::vector<std::tuple<std::string, std::string, double> > bb;
      comm.recv(bb, 0, 2014);
      for(auto & item : bb) {
        std::cout << "test" << std::get<0>(item) << "--" << std::get<1>(item) << "--" << std::get<2>(item) << std::endl;
      }
    }
  }

  { // another paracel list of triple send + recv
    if(rk == 0) {
      std::vector<std::tuple<std::string, std::string, double> > aa;
      std::tuple<std::string, std::string, double> tmp1;
      std::get<0>(tmp1) = "abc";
      std::get<1>(tmp1) = "def";
      std::get<2>(tmp1) = 4.15;
      aa.push_back(tmp1);
      std::tuple<std::string, std::string, double> tmp2;
      aa.push_back(tmp2);
      paracel::vrequest req;
      req = comm.isend(aa, 1, 2014);
    } else if(rk == 1) {
      std::vector<std::tuple<std::string, std::string, double> > bb;
      comm.recv(bb, 0, 2014);
      for(auto & item : bb) {
       std::cout << "check " << std::get<0>(item) << "--" << std::get<1>(item) << "--" << std::get<2>(item) << std::endl;
      }
    }
  }
  
  { // builtin sendrecv
    int a = 8;
    int b;
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(a, b, left, 2014, right, 2014);
    std::cout << "b" << b << std::endl;
  }

  { // container sendrecv
    std::vector<int> aaa{1,2,3};
    std::vector<int> bbb;
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(aaa, bbb, left, 2014, right, 2014);
    for(auto & item : bbb)
      std::cout << item << std::endl;
  }

  { // paracel triple sendrecv
    std::tuple<std::string, std::string, double> aa;
    std::tuple<std::string, std::string, double> bb;
    std::get<0>(aa) = "abc";
    std::get<1>(aa) = "def";
    std::get<2>(aa) = 3.14;
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(aa, bb, left, 2014, right, 2014);
    std::cout << "triple sendrecv" << std::get<0>(bb) << "--" << std::get<1>(bb) << "--" << std::get<2>(bb) << std::endl;
  }

  { // paracel list of triple sendrecv
    std::vector<std::tuple<std::string, std::string, double> > bb;
    std::vector<std::tuple<std::string, std::string, double> > aa;
    std::tuple<std::string, std::string, double> tmp1;
    std::get<0>(tmp1) = "abc";
    std::get<1>(tmp1) = "def";
    std::get<2>(tmp1) = 4.15;
    aa.push_back(tmp1);
    std::tuple<std::string, std::string, double> tmp2;
    std::get<0>(tmp2) = "cba";
    std::get<1>(tmp2) = "fed";
    std::get<2>(tmp2) = 5.16;
    aa.push_back(tmp2);
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(aa, bb, left, 2014, right, 2014);
    for(auto & item : bb) {
     std::cout << "test sendrecv" << std::get<0>(item) << "--" << std::get<1>(item) << "--" << std::get<2>(item) << std::endl;
    }
  }
  
  { // another paracel list of triple sendrecv
    std::vector<std::tuple<std::string, std::string, double> > bb;
    std::vector<std::tuple<std::string, std::string, double> > aa;
    std::tuple<std::string, std::string, double> tmp1;
    std::get<0>(tmp1) = "abc";
    std::get<1>(tmp1) = "def";
    std::get<2>(tmp1) = 4.15;
    std::tuple<std::string, std::string, double> tmp2;
    aa.push_back(tmp2);
    aa.push_back(tmp1);
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(aa, bb, left, 2014, right, 2014);
    for(auto & item : bb) {
     std::cout << "another test sendrecv" << std::get<0>(item) << "--" << std::get<1>(item) << "--" << std::get<2>(item) << std::endl;
    }
  }

  { // debug for list of triple sendrecv
    std::vector<std::vector<std::tuple<std::string, std::string, double> > > aa(2);
    std::vector<std::tuple<std::string, std::string, double> > bb;
    int t = 0, f = 0;
    if(rk == 0) {
      std::vector<std::tuple<std::string, std::string, double> > aaa;
      std::tuple<std::string, std::string, double> tmp1;
      std::get<0>(tmp1) = "a";
      std::get<1>(tmp1) = "b";
      std::get<2>(tmp1) = 1.;
      aaa.push_back(tmp1);
      std::get<0>(tmp1) = "a";
      std::get<1>(tmp1) = "c";
      std::get<2>(tmp1) = 1.;
      aaa.push_back(tmp1);
      std::get<0>(tmp1) = "a";
      std::get<1>(tmp1) = "d";
      std::get<2>(tmp1) = 1.;
      aaa.push_back(tmp1);
      std::get<0>(tmp1) = "b";
      std::get<1>(tmp1) = "a";
      std::get<2>(tmp1) = 1.;
      aaa.push_back(tmp1);
      aa[1] = aaa;
      t = 1;
      f = 1;
    } else if(rk == 1) {
      std::vector<std::tuple<std::string, std::string, double> > aaa;
      std::tuple<std::string, std::string, double> tmp1;
      std::get<0>(tmp1) = "e";
      std::get<1>(tmp1) = "a";
      std::get<2>(tmp1) = 1.;
      aaa.push_back(tmp1);
      std::get<0>(tmp1) = "e";
      std::get<1>(tmp1) = "d";
      std::get<2>(tmp1) = 1.;
      aaa.push_back(tmp1);
      aa[0] = aaa;

      std::vector<std::tuple<std::string, std::string, double> > aaaa;
      std::tuple<std::string, std::string, double> tmp2;
      std::get<0>(tmp2) = "b";
      std::get<1>(tmp2) = "d";
      std::get<2>(tmp2) = 1.;
      aaaa.push_back(tmp2);
      std::get<0>(tmp2) = "d";
      std::get<1>(tmp2) = "c";
      std::get<2>(tmp2) = 1.;
      aaaa.push_back(tmp2);
      aa[1] = aaaa;
      f = 0;
      t = 0;
    }
    comm.sendrecv(aa[t], bb, t, 2014, f, 2014);
  }

  { // builtin bcast
    int a;
    if(rk == 0) a = 7;
    comm.bcast(a, 0);
    std::cout << "rk " << rk << " a " << a << std::endl;
  }

  { // container bcast
    std::vector<int> aa(2);
    if(rk == 0) {
      aa[0] = 3;
      aa[1] = 4;
    }
    comm.bcast(aa, 0);
    std::cout << "rk " << rk << " aa0 " << aa[0] << " aa1 " << aa[1] << std::endl;
  }
  
  { // builtin alltoall
    std::vector<int> a(2), b(2);
    if(rk == 0) {
      a[0] = 1;
      a[1] = 3;
    }
    if(rk == 1) {
      a[0] = 2;
      a[1] = 4;
    }
    comm.alltoall(a, b);
    std::cout << " rk " << rk << b[0] << " " << b[1] << std::endl;
  }

  { // container alltoall
    std::vector< std::vector<int> > a(2), b;
    if(rk == 0) {
      std::vector<int> tmp1{1, 5};
      std::vector<int> tmp2{3};
      a[0] = tmp1;
      a[1] = tmp2;
    }
    if(rk == 1) {
      std::vector<int> tmp1{2};
      std::vector<int> tmp2{4, 7};
      a[0] = tmp1;
      a[1] = tmp2;
    }
    comm.alltoall(a, b);
    if(rk == 1) {
      for(auto & item : b[0])
        std::cout << " test " << item << " ";
      std::cout << "sep" << std::endl;
      for (auto & item : b[1])
        std::cout << " test " << item << " ";
      std::cout << std::endl;
    }
  }

  { // builtin allreduce
    int aaa;
    if(rk == 0) { aaa = 1; }
    if(rk == 1) { aaa = 2; }
    comm.allreduce(aaa);
    std::cout << " rk " << rk << " result " << aaa << std::endl;
  }
  
  { // container allreduce
    std::vector<int> aaa(3);
    if(rk == 0) {
      aaa[0] = 1;
      aaa[1] = 2;
      aaa[2] = 3;
    }
    if(rk == 1) {
      aaa[0] = 3;
      aaa[1] = 2;
      aaa[2] = 1;
    }
    comm.allreduce(aaa);
    for(auto & item : aaa)
      std::cout << " rk " << rk << " result aaa " << item << std::endl;
  }

  { // bcastring
    std::vector<int> a;
    if(rk == 0) {
      a.push_back(6);
      a.push_back(42);
    }
    if(rk == 1) {
      a.push_back(28);
      a.push_back(42);
      a.push_back(42);
      a.push_back(28);
      a.push_back(6);
    }
    std::set<int> result;
    auto func = [&](std::vector<int> tmp){
      for(auto & stf : tmp)
        result.insert(stf);
    };
    comm.bcastring(a, func);
    for(auto & item : result) {
      std::cout << rk << " : "<< item << std::endl;
    }
  }

  { // dict_type<size_t, int> isend
    if(rk == 0) {
      std::unordered_map<size_t, int> aa;
      aa[0] = 1;
      aa[1] = 2;
      paracel::vrequest req;
      req = comm.isend(aa, 1, 2014);
    } else if(rk == 1) {
      std::unordered_map<size_t, int> bb;
      comm.recv(bb, 0, 2014);
      for(auto & item : bb)
        std::cout << item.first << " * " << item.second << std::endl;
    }
  }

  { // alltoall
  }

  return 0;
}
