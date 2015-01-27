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

#ifndef FILE_ec8e1787_f407_c643_5d12_b8c93bdb52bb_HPP 
#define FILE_ec8e1787_f407_c643_5d12_b8c93bdb52bb_HPP 

#include <sstream>
#include <algorithm> // std::sort, std::find
#include <functional>

#include "paracel_types.hpp"
#include "utils.hpp"
#include "utils/hash.hpp"

namespace paracel {

// T rep type of server name
template <class T>
class ring {

public:
 
  ring(paracel::list_type<T> names) {
    for(auto & name : names) {
      add_server(name);
    }
  }

  ring(paracel::list_type<T> names, int cp) : replicas(cp) {
    for(auto & name : names) {
      add_server(name);
    }
  }

  void add_server(const T & name) {
    //std::hash<paracel::str_type> hfunc;
    paracel::hash_type<paracel::str_type> hfunc;
    std::ostringstream tmp;
    tmp << name;
    auto name_str = tmp.str();
    for(int i = 0; i < replicas; ++i) {
      std::ostringstream cvt;
      cvt << i;
      auto n = name_str + ":" + cvt.str();
      auto key = hfunc(n);
      srv_hashring_dct[key] = name;
      srv_hashring.push_back(key);
    }
    // sort srv_hashring
    std::sort(srv_hashring.begin(), srv_hashring.end());
  }

  void remove_server(const T & name) {
    //std::hash<paracel::str_type> hfunc;
    paracel::hash_type<paracel::str_type> hfunc;
    std::ostringstream tmp;
    tmp << name;
    auto name_str = tmp.str();
    for(int i = 0; i < replicas; ++i) {
      std::ostringstream cvt;
      cvt << i;
      auto n = name_str + ":" + cvt.str();
      auto key = hfunc(n);
      srv_hashring_dct.erase(key);
      auto iter = std::find(srv_hashring.begin(), srv_hashring.end(), key);
      if(iter != srv_hashring.end()) {
        srv_hashring.erase(iter);
      }
    }
  }

  // TODO: relief load of srv_hashring_dct[srv_hashring[0]]
  template <class P>
  T get_server(const P & skey) {
    //std::hash<P> hfunc;
    paracel::hash_type<P> hfunc;
    auto key = hfunc(skey);
    auto server = srv_hashring[paracel::ring_bsearch(srv_hashring, key)];
    return srv_hashring_dct[server];
  }

private:
  int replicas = 32;
  paracel::list_type<paracel::hash_return_type> srv_hashring;
  paracel::dict_type<paracel::hash_return_type, T> srv_hashring_dct;
};

} // namespace paracel

#endif
