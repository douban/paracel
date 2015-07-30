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

#ifndef FILE_0c75247e_03c0_5a81_3776_1d686062eb51_HPP
#define FILE_0c75247e_03c0_5a81_3776_1d686062eb51_HPP

#include <vector>

//#include <tr1/unordered_map>
#include <boost/optional/optional.hpp>

#include "paracel_types.hpp"
#include "utils.hpp"

namespace paracel {

template <class K, class V>
struct kvs {

public:

  virtual ~kvs() {}
  
  bool contains(const K & k) { 
    return kvdct.count(k); 
  }
  
  void set(const K & k, const V & v) { 
    kvdct[k] = v; 
  }

  void set_multi(const paracel::dict_type<K, V> & kvdict) {
    for(auto & kv : kvdict) {
      set(kv.first, kv.second);
    }
  }

  /*
  // another impl, more mem will be used
  paracel_list_type<V> get(const K & k) {
    parace_list_type tmp_val_list;
    if(contains(k)) {
      tmp_val_list.push_back(kvdct.at(k));
    } 
    return tmp_val_list; 
  }
  */

  boost::optional<V> get(const K & k) {
    auto fi = kvdct.find(k);
    if(fi != kvdct.end()) {
      return boost::optional<V>(fi->second);
    } else return boost::none;
  }

  bool get(const K & k, V & v) {
    auto fi = kvdct.find(k);
    if(fi != kvdct.end()) {
      v = fi->second;
      return true;
    } else {
      return false;
    }
  }

  paracel::list_type<V> 
  get_multi(const paracel::list_type<K> & keylst) {
    paracel::list_type<V> valst;
    for(auto & key : keylst) {
      //try {
      valst.push_back(kvdct.at(key));
      //} catch (std::out_of_range& oor) {
      //  ERROR_PRINT(oor, "get_multi error is caught");
      //  throw oor;
      //}
    }
    return valst;
  }

  void get_multi(const paracel::list_type<K> & keylst,
                 paracel::list_type<V> & valst) {
    for(auto & key : keylst) {
      //try {
      valst.push_back(kvdct.at(key));
      //} catch (std::out_of_range& oor) {
      //  ERROR_PRINT(oor, "get_multi error is caught");
      //  throw oor;
      //}
    }
  }

  void get_multi(const paracel::list_type<K> & keylst,
                 paracel::dict_type<K, V> & valdct) {
    valdct.clear();
    for(auto & key : keylst) {
      auto it = kvdct.find(key);
      if(it != kvdct.end()) {
        valdct[key] = it->second;
      }
    }
  }
  
  // tricky: T is equal to V actually, but SFINAE works here. so use template 
  template <class T> 
  paracel::Enable_if<paracel::is_atomic<T>::value, bool>
  incr(const K & k, const T & delta) {
    if(!contains(k)) return false;
    kvdct[k] += delta;
    return true;
  }

  template <class T>
  paracel::Enable_if<paracel::is_seqic<T>::value, bool>
  incr(const K & k, const T & delta) {
    if(!contains(k)) return false;
    if(kvdct[k].size() != delta.size()) return false;
    for(size_t i = 0; i < delta.size(); ++i) {
      kvdct[k][i] += delta[i];
    }
    return true;
  }

  // gets(key) -> value, unique
  boost::optional<std::pair<V, paracel::hash_return_type> >
  gets(const K & k) {
    if(auto v = get(k)) {
      std::pair<V, paracel::hash_return_type> ret(*v, hfunc(*v));
      return boost::optional<
          std::pair<V, paracel::hash_return_type> 
			      >(ret);
    } else {
      return boost::none;
    }
  }

  // compare-and-set, cas(key, value, unique) -> True/False
  bool cas(const K & k, const V & v, const paracel::hash_return_type & uniq) {
    if(auto r = gets(k)) {
      if(uniq == (*r).second) {
        set(k, v);
        return true;
      } else {
        return false;
      }
    } else {
      kvdct[k] = v;
    }
    return true; 
  }

  bool del(const K & k) {
    if(!contains(k)) {
      return false;
    }
    kvdct.erase(k);
    return true;
  }
  
  void clean() {
    kvdct.clear();
  }
  
  paracel::dict_type<K, V> getall() {
    return kvdct;
  }

private:
  //std::tr1::unordered_map<K, V> kvdct;
  paracel::dict_type<K, V> kvdct;
  paracel::hash_type<V> hfunc;
};

} // namespace paracel

#endif
