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

#ifndef FILE_50cbcab2_09cc_7016_42c0_609317d6df63_HPP
#define FILE_50cbcab2_09cc_7016_42c0_609317d6df63_HPP

#include <assert.h>
#include <dlfcn.h>

#include <set>
#include <tuple>
#include <queue>
#include <fstream>
#include <utility>
#include <future>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>

#include <boost/any.hpp>
#include <boost/filesystem.hpp>

#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/Dense>

#include "load.hpp"
#include "ring.hpp"
#include "graph.hpp"
#include "utils.hpp"
#include "packer.hpp"
#include "client.hpp"
#include "paracel_types.hpp"

namespace paracel {

using parser_type = std::function<paracel::list_type<paracel::str_type>(paracel::str_type)>;

class paralg {

 private:
  void init_output(const paracel::str_type & folder) {
    if(folder.size() == 0) return;
    // create output folder in advance
    if(worker_comm.get_rank() == 0) {
      boost::filesystem::path path(folder);
      if(boost::filesystem::exists(path)) {
        //boost::filesystem::remove_all(path);
        return;
      }
      boost::filesystem::create_directories(path);
    }
  }

  // TODO: abstract update_f as para
  void load_update_f(const paracel::str_type & fn,
                     const paracel::str_type & fcn) {
    void *handler = dlopen(fn.c_str(), RTLD_NOW |
                           RTLD_LOCAL |
                           RTLD_NODELETE);
    if(!handler) {
      std::cerr << "Cannot open library: " << dlerror() << '\n';
      return;
    }
    auto local = dlsym(handler, fcn.c_str());
    if(!local) {
      std::cerr << "Cannot load symbol: " << dlerror() << '\n';
      dlclose(handler);
      return;
    }
    update_f = *(std::function<paracel::str_type(paracel::str_type, paracel::str_type)>*) local;
    dlclose(handler);
  }

 public:
  // constructor for direct usage
  paralg(paracel::Comm comm,
         paracel::str_type _output = "",
         int _rounds = 1) : worker_comm(comm), 
	                    output(_output),
			    rounds(_rounds) {
    init_output(_output);
    clock = 0;
    stale_cache = 0;
    clock_server = 0;
    total_iters = rounds;
    ps_obj = NULL;
  }

  paralg(paracel::str_type hosts_dct_str, 
         paracel::Comm comm,
         paracel::str_type _output = "",
         int _rounds = 1,
         int _limit_s = 0,
         bool _ssp_switch = false) : worker_comm(comm),
                                    output(_output),
                                    nworker(comm.get_size()),
                                    rounds(_rounds),
                                    limit_s(_limit_s),
                                    ssp_switch(_ssp_switch) {
    ps_obj = new parasrv(hosts_dct_str);
    init_output(_output);
    clock = 0;
    stale_cache = 0;
    clock_server = 0;
    total_iters = rounds;
    if(worker_comm.get_rank() == 0) {
      paracel::str_type key = "worker_sz";
      (ps_obj->kvm[clock_server]).
          push_int(key, worker_comm.get_size());
    }
    paracel_sync();
  }

  virtual ~paralg() {
    if(ps_obj) {
      delete ps_obj;
    }
  }

  void set_decomp_info(const paracel::str_type & pattern) {
    int np = worker_comm.get_size();
    paracel::npfactx(np, npx, npy);
    if(pattern == "fsmap") {
      paracel::npfact2d(np, npx, npy);
    }
    if(pattern == "smap") {
      paracel::npfacty(np, npx, npy); 
    }
  }

  template <class T>
  paracel::list_type<paracel::str_type> 
  paracel_loadall(const T & fn) {
    auto fname_lst = paracel::expand(fn);
    paracel::list_type<paracel::str_type> lines;
    for(auto & fname : fname_lst) {
      std::ifstream f(fname, std::ios::binary);
      if(!f) { 
        throw std::runtime_error("internal error in paracel_loadall: loader reading failed.");
      }
      paracel::str_type l;
      while(std::getline(f, l)) {
        lines.push_back(l);
      }
      f.close();
    }
    return lines;
  }

  template <class T, class F>
  void paracel_sequential_loadall(const T & fn, F & func) {
    
    auto fname_lst = paracel::expand(fn);
    paracel::partition partition_obj(fname_lst, get_worker_size(), "linesplit");
    partition_obj.files_partition();
    auto slst = partition_obj.get_start_list();
    auto elst = partition_obj.get_end_list();
    // sequential_load
    for(size_t i = 0; i < get_worker_size(); ++i) {
      auto lines = partition_obj.files_load_lines_impl(slst[i], elst[i]);
      func(lines);
    }
  }

  template <class T>
  paracel::list_type<paracel::str_type> 
  paracel_load(const T & fn,
               parser_type & parser) {
    paracel::loader<T> ld(fn, worker_comm,
                          parser,
                          "linesplit",
                          false);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    set_decomp_info("linesplit");
    //assert(lines.size() != 0);
    return lines;
  }

  template <class T>
  paracel::list_type<paracel::str_type> 
  paracel_load(const T & fn) {
    parser_type parser;
    return paralg::paracel_load(fn, parser);	
  }

  template <class T, class F>
  void paracel_load_handle(const T & fn,
                           F func,
                           const paracel::str_type & pattern = "linesplit",
                           bool mix_flag = false) {
    paracel::loader<T> ld(fn, worker_comm, pattern);
    ld.fixload_handle(func);
    set_decomp_info(pattern);
  }

  // only support paracel::digraph<paracel::default_id_type> and paracel::digraph<std::string>
  template <class T, class G>
  void paracel_load_as_graph(paracel::digraph<G> & grp,
                             const T & fn, 
                             parser_type & parser,
                             const paracel::str_type & pattern = "fmap",
                             bool mix_flag = false) {
    // TODO: check pattern 
    if(pattern == "fset") {
      mix_flag = true;
    }
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    paracel_sync();
    // create graph 
    ld.create_graph(lines, grp);
    set_decomp_info(pattern);
    lines.resize(0); lines.shrink_to_fit(); paracel::cheat_to_os();
  }

  // only support paracel::bigraph<paracel::default_id_type> and paracel::bigraph<std::string>
  template <class T, class G>
  void paracel_load_as_graph(paracel::bigraph<G> & grp,
                             const T & fn,
                             parser_type & parser,
                             const paracel::str_type & pattern = "fmap",
                             bool mix_flag = false) {
    if(pattern == "fset") {
      mix_flag = true;
    }
    // TODO: check pattern 
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    paracel_sync();
    // create graph 
    ld.create_graph(lines, grp);
    set_decomp_info(pattern);
    lines.resize(0); lines.shrink_to_fit(); paracel::cheat_to_os();
  }

  template <class T>
  void paracel_load_as_graph(paracel::bigraph_continuous & grp,
                             paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & row_map,
                             paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & col_map,
                             const T & fn,
                             parser_type & parser,
                             const paracel::str_type & pattern = "fmap",
                             bool mix_flag = false) {
    // TODO: check pattern 
    if(pattern == "fset") {
      mix_flag = true;
    }
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    paracel_sync();
    // create graph 
    ld.create_graph(lines, grp, row_map, col_map);
    set_decomp_info(pattern);
    lines.resize(0); lines.shrink_to_fit(); paracel::cheat_to_os();
  }

  template <class T, class G>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
                              paracel::dict_type<paracel::default_id_type, G> & row_map,
                              paracel::dict_type<paracel::default_id_type, G> & col_map,
                              const T & fn, 
                              parser_type & parser,
                              const paracel::str_type & pattern = "fsmap",
                              bool mix_flag = false) {
    // TODO: check pattern
    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, pattern, mix_flag);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    paracel_sync();
    // create sparse matrix
    ld.create_matrix(lines, blk_mtx, row_map, col_map);
    set_decomp_info(pattern);
    lines.resize(0); lines.shrink_to_fit(); paracel::cheat_to_os();
  }
  
  template <class T, class G>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
                              paracel::dict_type<paracel::default_id_type, G> & row_map,
                              const T & fn, 
                              parser_type & parser,
                              const paracel::str_type & pattern = "fsmap",
                              bool mix_flag = false) {
    paracel::dict_type<paracel::default_id_type, G> col_map;
    paralg::paracel_load_as_matrix(blk_mtx, 
                                   row_map, col_map, 
                                   fn, parser, pattern, mix_flag);
  }

  // simple interface
  template <class T>
  void paracel_load_as_matrix(Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
                              const T & fn, 
                              parser_type & parser,
                              const paracel::str_type & pattern = "fsmap",
                              bool mix_flag = false) {
    return paralg::paracel_load_as_matrix(blk_mtx, rm, cm,
                                          fn, parser, pattern, mix_flag);
  }

  template <class T, class G>
  void paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,
                              paracel::dict_type<paracel::default_id_type, G> & row_map,
                              const T & fn, 
                              parser_type & parser) {

    // load lines
    paracel::loader<T> ld(fn, worker_comm, parser, "fvec", true);
    paracel::list_type<paracel::str_type> lines = ld.fixload();
    paracel_sync();
    // create sparse matrix
    ld.create_matrix(lines, blk_dense_mtx, row_map);
    set_decomp_info("fvec");
    lines.resize(0); lines.shrink_to_fit(); paracel::cheat_to_os();
  }

  // simple interface
  template <class T>
  void paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,
                              const T & fn, 
                              parser_type & parser) {
    paralg::paracel_load_as_matrix(blk_dense_mtx,
                                   rm, fn,
                                   parser);	
  }

  // put where you want to control iter with ssp
  void iter_commit() {
    paracel::str_type clock_key;
    if(limit_s == 0) {
      clock_key = "client_clock_0";
    } else {
      clock_key = "client_clock_" + std::to_string(clock % limit_s);
    }
    ps_obj->kvm[clock_server].incr_int(paracel::str_type(clock_key), 1); // value 1 is not important
    clock += 1;
    if(clock == total_iters) {
      ps_obj->kvm[clock_server].incr_int(paracel::str_type("worker_sz"), -1);
    }
  }

  void get_decomp_info(int & x, int & y) {
    x = npx;
    y = npy;
  }

  bool paracel_register_update(const paracel::str_type & file_name,
                               const paracel::str_type & func_name) {
    load_update_f(file_name, func_name);
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_update(file_name, func_name);
    }
    return r;
  }

  bool paracel_register_bupdate(const paracel::str_type & file_name,
                                const paracel::str_type & func_name) {
    //local_update_f(file_name, func_name);
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_bupdate(file_name, func_name);
    }
    return r;
  }

  bool paracel_register_read_special(const paracel::str_type & file_name,
                                     const paracel::str_type & func_name) {
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_pullall_special(file_name,
                                                       func_name);
    }
    return r;
  }

  bool paracel_register_remove_special(const paracel::str_type & file_name,
                                       const paracel::str_type & func_name) {
    bool r = true;
    for(int i = 0; i < ps_obj->srv_sz; ++i) {
      r = r && ps_obj->kvm[i].register_remove_special(file_name, func_name);
    }
    return r;
  }

  template <class V>
  bool paracel_read(const paracel::str_type & key,
                    V & val,
                    int replica_id = -1) {
    if(ssp_switch) {
      /*
         std::cout << "--------------" << std::endl;
         std::cout << get_worker_id() << "stale_cache:" << stale_cache << std::endl;
         std::cout << get_worker_id() << "clock" << clock << std::endl;
         std::cout << "--------------" << std::endl;
      */  
      if(clock == 0 || clock == total_iters) { // check total_iters for last pull
        cached_para[key] = boost::any_cast<V>(ps_obj->
                                              kvm[ps_obj->p_ring->get_server(key)].
                                              pull<V>(key));
        val = boost::any_cast<V>(cached_para[key]);
      } else if(stale_cache + limit_s > clock) {
        // cache hit
        val = boost::any_cast<V>(cached_para[key]);
      } else {
        // cache miss
        // pull from server until leading slowest less than s clocks
        while(stale_cache + limit_s < clock) {
          stale_cache = ps_obj->
              kvm[clock_server].pull_int(paracel::str_type("server_clock"));
        }
        cached_para[key] = boost::any_cast<V>(ps_obj->
                                              kvm[ps_obj->p_ring->get_server(key)].
                                              pull<V>(key));
        val = boost::any_cast<V>(cached_para[key]);
      }
      return true;
    }
    return ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull(key, val); 
  }

  template <class V>
  V paracel_read(const paracel::str_type & key,
                 int replica_id = -1) {
    if(ssp_switch) {
      V val;
      if(clock == 0 || clock == total_iters) {
        cached_para[key] = boost::any_cast<V>(ps_obj->
                                              kvm[ps_obj->p_ring->get_server(key)].
                                              pull<V>(key));
        val = boost::any_cast<V>(cached_para[key]);
      } else if(stale_cache + limit_s > clock) {
        val = boost::any_cast<V>(cached_para[key]);
      } else {
        while(stale_cache + limit_s < clock) {
          stale_cache = ps_obj->
              kvm[clock_server].pull_int(paracel::str_type("server_clock"));
        }
        cached_para[key] = boost::any_cast<V>(ps_obj->
                                              kvm[ps_obj->p_ring->get_server(key)].
                                              pull<V>(key));
        val = boost::any_cast<V>(cached_para[key]);
      }
      return val;
    }
    return ps_obj->kvm[ps_obj->p_ring->get_server(key)].pull<V>(key);
  }

  template <class V>
  void paracel_read_multi(const paracel::list_type<paracel::str_type> & keys,
                          paracel::dict_type<paracel::str_type, V> & vals) {
    vals.clear();
    paracel::list_type<paracel::list_type<paracel::str_type> > lst_lst(ps_obj->srv_sz);
    for(size_t k = 0; k < keys.size(); ++k) {
      lst_lst[ps_obj->p_ring->get_server(keys[k])].push_back(keys[k]);
    }
    if(ssp_switch) {
      // TODO
    }
    for(size_t k = 0; k < lst_lst.size(); ++k) {
      paracel::dict_type<paracel::str_type, V> dct;
      ps_obj->kvm[k].pull_multi(lst_lst[k], dct);
      vals.insert(dct.begin(), dct.end());
    }
  }

  template<class V>
  paracel::list_type<V> 
  paracel_read_multi(const paracel::list_type<paracel::str_type> & keys) {
    paracel::list_type<V> vals;
    paracel::dict_type<paracel::str_type, size_t> indx_map;
    paracel::list_type<paracel::list_type<paracel::str_type> > lst_lst(ps_obj->srv_sz);
    vals.resize(keys.size());
    for(size_t k = 0; k < keys.size(); ++k) {
      lst_lst[ps_obj->p_ring->get_server(keys[k])].push_back(keys[k]);
      indx_map[keys[k]] = k;
    }
    
    if(ssp_switch) {
      if(clock == 0 || clock == total_iters) {
        for(size_t k = 0; k < lst_lst.size(); ++k) {
          auto lst = ps_obj->kvm[k].pull_multi<V>(lst_lst[k]);
          for(size_t i = 0; i < lst.size(); ++i) {
            vals[indx_map[lst_lst[k][i]]] = lst[i];
          }
        }
        for(size_t i = 0; i < vals.size(); ++i) {
          cached_para[keys[i]] = boost::any_cast<V>(vals[i]);
        }
      } else if(stale_cache + limit_s > clock) {
        for(size_t i = 0; i < vals.size(); ++i) {
          vals[i] = boost::any_cast<V>(cached_para[keys[i]]);
        }
      } else {
        while(stale_cache + limit_s < clock) {
          stale_cache = ps_obj->
              kvm[clock_server].pull_int(paracel::str_type("server_clock"));
        }
        for(size_t k = 0; k < lst_lst.size(); ++k) {
          auto lst = ps_obj->kvm[k].pull_multi<V>(lst_lst[k]);
          for(size_t i = 0; i < lst.size(); ++i) {
            vals[indx_map[lst_lst[k][i]]] = lst[i];
          }
        }
        for(size_t i = 0; i < vals.size(); ++i) {
          cached_para[keys[i]] = boost::any_cast<V>(vals[i]);
        }
      }
      return vals;
    } // ssp_switch
    
    for(size_t k = 0; k < lst_lst.size(); ++k) {
      auto lst = ps_obj->kvm[k].pull_multi<V>(lst_lst[k]);
      for(size_t i = 0; i < lst.size(); ++i) {
        vals[indx_map[lst_lst[k][i]]] = lst[i];
      }
    }
    return vals;
  }

  // TODO
  template<class V>
  paracel::dict_type<paracel::str_type, V> paracel_readall() {
    paracel::dict_type<paracel::str_type, V> d;
    for(int indx = 0; indx < ps_obj->srv_sz; ++indx) {
      auto tmp = ps_obj->kvm[indx].pullall<V>();
      for(auto & kv : tmp) {
        d[kv.first] = kv.second;
      }
    }
    return d;
  }
  
  template<class V, class F>
  void paracel_readall_handle(F & func) {
    for(int indx = 0; indx < ps_obj->srv_sz; ++indx) {
      paracel::dict_type<paracel::str_type, V> d;
      auto tmp = ps_obj->kvm[indx].pullall<V>();
      for(auto & kv : tmp) {
        d[kv.first] = kv.second;
      }
      func(d);
    }
  }

  template <class V>
  paracel::dict_type<paracel::str_type, V>
  paracel_read_special(const paracel::str_type & file_name,
                       const paracel::str_type & func_name) {
    paracel::dict_type<paracel::str_type, V> d;
    for(int indx = 0; indx < ps_obj->srv_sz; ++indx) {
      auto tmp = ps_obj->kvm[indx].pullall_special<V>(file_name, func_name);
      for(auto & kv : tmp) {
        d[kv.first] = kv.second;
      }
    }
    return d;
  }

  template <class V, class F>
  void paracel_read_special_handle(const paracel::str_type & file_name,
                                   const paracel::str_type & func_name,
                                   F & func) {
    for(int indx = 0; indx < ps_obj->srv_sz; ++indx) {
      paracel::dict_type<paracel::str_type, V> d;
      auto tmp = ps_obj->kvm[indx].pullall_special<V>(file_name, func_name);
      for(auto & kv : tmp) {
        d[kv.first] = kv.second;
      }
      func(d);
    }
  }

  /*
   * risk: if you use this interface, params in server must all be the same type
   */
  template <class T>
  void paracel_read_topk(int k, paracel::list_type<
                              std::pair<paracel::str_type, T> 
                              > & result) {
    using min_heap = std::priority_queue<std::pair<paracel::str_type, T>, 
                                        std::vector<std::pair<paracel::str_type, T> >, 
                                        min_heap_cmp<T> >;
    min_heap tmplst;
    auto handler = [&tmplst, k] (const paracel::dict_type<paracel::str_type, T> & d) {
      for(auto & kv : d) {
        auto node = paracel::heap_node<paracel::str_type, T>(kv.first, kv.second);
        tmplst.push(node.val);
        if((int)tmplst.size() > k) {
          tmplst.pop();
        }
      }
    }; // handler
    paracel_readall_handle<T>(handler);
    result.resize(0);
    while(!tmplst.empty()) {
      result.push_back(std::make_pair(tmplst.top().first, tmplst.top().second));
      tmplst.pop();
    }
    std::reverse(result.begin(), result.end());
  }

  template <class T>
  void paracel_read_topk_with_key_filter(int k, 
                                         paracel::list_type<
                                          std::pair<paracel::str_type, T>
                                          > & result,
                                         const paracel::str_type & file_name,
                                         const paracel::str_type & func_name) {
    using min_heap = std::priority_queue<std::pair<paracel::str_type, T>,
                                        std::vector<std::pair<paracel::str_type, T> >,
                                        min_heap_cmp<T> >;
    min_heap tmplst;
    auto handler = [&tmplst, k] (const paracel::dict_type<paracel::str_type, T> & d) {
      for(auto & kv : d) {
        auto node = paracel::heap_node<paracel::str_type, T>(kv.first, kv.second);
        tmplst.push(node.val);
        if((int)tmplst.size() > k) {
          tmplst.pop();
        }
      }
    }; // handler
    paracel_read_special_handle<T>(file_name, func_name, handler);
    result.resize(0);
    while(!tmplst.empty()) {
      result.push_back(std::make_pair(tmplst.top().first,
                                      tmplst.top().second));
      tmplst.pop();
    }
    std::reverse(result.begin(), result.end());
  }
  
  template <class V>
  bool paracel_write(const paracel::str_type & key,
                     const V & val,
                     bool replica_flag = false) {
    auto indx = ps_obj->p_ring->get_server(key);
    if(ssp_switch) {
      cached_para[key] = boost::any_cast<V>(val);
    }
    return (ps_obj->kvm[indx]).push(key, val);
  }

  bool paracel_write(const paracel::str_type & key,
                     const char* val,
                     bool replica_flag = false) {
    paracel::str_type v = val;
    return paralg::paracel_write(key, v); 
  }

  // TODO: package
  template <class V>
  bool paracel_write_multi(const paracel::dict_type<paracel::str_type, V> & dct) {
    if(ssp_switch) {
      for(auto & kv : dct) {
        cached_para[kv.first] = boost::any_cast<V>(kv.second);
      }
    }
    bool r = true;
    paracel::list_type<paracel::dict_type<paracel::str_type, V> > dct_lst(ps_obj->srv_sz);
    for(auto & kv : dct) {
      dct_lst[ps_obj->p_ring->get_server(kv.first)][kv.first] = kv.second;
    }
    for(size_t k = 0; k < dct_lst.size(); ++k) {
      if(dct_lst[k].size() != 0) {
        if(ps_obj->kvm[k].push_multi(dct_lst[k]) == false) {
          r = false;
        }
      }
    }
    return r;
  }
  
  template <class V>
  void paracel_update(const paracel::str_type & key,
                      const V & delta,
                      paracel::async_functor_type & update_future,
                      bool replica_flag = false) {
    if(ssp_switch) {
      if(!update_f) {
        // load default updater
        load_update_f("../local/build/lib/default.so",
                      "default_incr_d");
      }
      V val = boost::any_cast<V>(cached_para[key]);
      // pack<V> val to v & delta to d
      paracel::packer<V> pk1(val), pk2(delta);
      paracel::str_type v, d; pk1.pack(v); pk2.pack(d);
      auto nv = update_f(v, d);
      // unpack<V> nv
      V nval = pk1.unpack(nv);
      cached_para[key] = boost::any_cast<V>(nval);
    }
    ps_obj->kvm[ps_obj->p_ring->get_server(key)].update(key, delta, update_future);
  }

  void paracel_update(const paracel::str_type & key,
                      const char* delta,
                      paracel::async_functor_type & update_future,
                      bool replica_flag = false) {
    paracel::str_type d = delta;
    paralg::paracel_update(key, d, update_future);
  }
  
  template <class V>
  void paracel_update(const paracel::str_type & key,
                      const V & delta,
                      const paracel::str_type & file_name,
                      const paracel::str_type & func_name,
                      paracel::async_functor_type & update_future) {
    if(ssp_switch) {
      V val = boost::any_cast<V>(cached_para[key]);
      // pack<V> val to v & delta to d
      paracel::packer<V> pk1(val), pk2(delta);
      paracel::str_type v, d; pk1.pack(v); pk2.pack(d);
      auto nv = update_f(v, d);
      // unpack<V> nv
      V nval = pk1.unpack(nv);
      cached_para[key] = boost::any_cast<V>(nval);
    }
    ps_obj->kvm[ps_obj->p_ring->get_server(key)].update(key,
                                                        delta,
                                                        file_name,
                                                        func_name,
                                                        update_future);
  }

  void paracel_update(const paracel::str_type & key,
                      const char* delta,
                      const paracel::str_type & file_name,
                      const paracel::str_type & func_name,
                      paracel::async_functor_type & update_future) {
    paracel::str_type d = delta;
    paralg::paracel_update(key, d, file_name, func_name, update_future);
  }


  template <class V>
  bool paracel_bupdate(const paracel::str_type & key,
                       const V & delta,
                       bool replica_flag = false) {
    int indx = ps_obj->p_ring->get_server(key);
    bool r = false;
    auto new_val = ps_obj->kvm[indx].bupdate(key, delta, r);
    if(ssp_switch) {
      // update local cache
      cached_para[key] = boost::any_cast<V>(new_val);
      /*
      cached_para[key] = boost::any_cast<V>(ps_obj->
                                            kvm[indx].
                                            pull<V>(key));
      */
    }
    return r;
  }

  bool paracel_bupdate(const paracel::str_type & key,
                       const char* delta,
                       bool replica_flag = false) {
    paracel::str_type d = delta;
    return paralg::paracel_bupdate(key, d, replica_flag);
  }

  template <class V>
  bool paracel_bupdate(const paracel::str_type & key, 
                       const V & delta,
                       const paracel::str_type & file_name, 
                       const paracel::str_type & func_name,
                       bool replica_flag = false) {
    int indx = ps_obj->p_ring->get_server(key);
    bool r = false;
    auto new_val = ps_obj->kvm[indx].bupdate(key,
                                             delta,
                                             file_name,
                                             func_name,
                                             r);
    if(ssp_switch) {
      // update local cache
      cached_para[key] = boost::any_cast<V>(new_val);
      //cached_para[key] = boost::any_cast<V>(ps_obj->kvm[indx].pull<V>(key));
    }
    return r;
  }

  bool paracel_bupdate(const paracel::str_type & key, 
                       const char* delta, 
                       const paracel::str_type & file_name, 
                       const paracel::str_type & func_name,
                       bool replica_flag = false) {
    paracel::str_type d = delta;
    return paralg::paracel_bupdate(key, d, file_name, func_name, replica_flag);
  }

  // TODO
  template <class V>
  bool paracel_bupdate_multi(const paracel::list_type<paracel::str_type> & keys,
                             const paracel::list_type<V> & deltas,
                             const paracel::str_type & file_name,
                             const paracel::str_type & func_name) {
    paracel::list_type<std::pair<paracel::list_type<paracel::str_type>,
                                paracel::list_type<V> > > kd_lst(ps_obj->srv_sz);
    bool r = true;
    for(size_t i = 0; i < keys.size(); ++i) {
      auto indx = ps_obj->p_ring->get_server(keys[i]);
      kd_lst[indx].first.push_back(keys[i]);
      kd_lst[indx].second.push_back(deltas[i]);
    }
    for(size_t k = 0; k < kd_lst.size(); ++k) {
      auto key_lst = kd_lst[k].first;
      auto delta_lst = kd_lst[k].second;
      bool rr = false;
      auto tmp = ps_obj->kvm[k].bupdate_multi(key_lst,
                                              delta_lst,
                                              file_name,
                                              func_name,
                                              rr);
      if(rr == false) r = false;
      if(ssp_switch) {
        for(size_t j = 0; j < key_lst.size(); ++j) {
          cached_para[key_lst[j]] = boost::any_cast<V>(tmp[j]);
        }
      }
    }
    return r;
  }

  template <class V>
  bool paracel_bupdate_multi(const paracel::dict_type<paracel::str_type, V> & dct,
                             const paracel::str_type & file_name,
                             const paracel::str_type & func_name) {
    bool r = true;
    paracel::list_type<paracel::dict_type<paracel::str_type, V> > dct_lst(ps_obj->srv_sz);
    for(auto & kv : dct) {
      dct_lst[ps_obj->p_ring->get_server(kv.first)][kv.first] = kv.second;
    }
    for(size_t k = 0; k < dct_lst.size(); ++k) {
      if(dct_lst[k].size() != 0) {
        bool rr = false;
        auto tmp = ps_obj->kvm[k].bupdate_multi(dct_lst[k],
                                                file_name,
                                                func_name,
                                                rr);
        if(rr == false) r = false;
        if(ssp_switch) {
          paracel::list_type<paracel::str_type> tmp_lst;
          for(auto & kv : dct_lst[k]) {
            tmp_lst.push_back(kv.first);
          }
          for(size_t j = 0; j < tmp_lst.size(); ++j) {
            cached_para[tmp_lst[j]] = boost::any_cast<V>(tmp[j]);
          }
        } // ssp_switch
      }
    }
    return r;
  }

  // set invoke cnts
  void set_total_iters(int n) {
    total_iters = n;
  }

  size_t get_worker_id() {
    return worker_comm.get_rank();
  }

  size_t get_worker_size() {
    return worker_comm.get_size();
  }

  size_t get_server_size() {
    return ps_obj->srv_sz;
  }

  void paracel_sync() {
    worker_comm.synchronize();
  }

  /**
   * Never called in if(rank == 0) clause because dup will hang.
   */
  paracel::Comm get_comm() {
    return worker_comm;
  }

  boost::any get_cache() {
    return cached_para;
  }

  bool is_cached(const paracel::str_type & key) {
    return cached_para.count(key);
  }

  template <class V>
  V get_cache(const paracel::str_type & key) {
    return boost::any_cast<V>(cached_para.at(key));
  }

  bool paracel_contains(const paracel::str_type & key) {
    auto indx = ps_obj->p_ring->get_server(key);
    return (ps_obj->kvm[indx]).contains(key);
  }

  bool paracel_remove(const paracel::str_type & key) {
    auto indx = ps_obj->p_ring->get_server(key);
    return ps_obj->kvm[indx].remove(key);
  }

  bool paracel_remove_multi(const paracel::list_type<paracel::str_type> & key_lst) {
    // TODO
    return true;
  }

  template <class T>
  void pkl_dat(const T & m, std::string prefix = "tmp") {
    paracel::pkl_dat_sequential(m,
                                prefix + std::to_string(worker_comm.get_rank()) + ".pkl");
  }

  // TODO
  template <class T, class DAT>
  void unpkl_dat(const T & fn, DAT & dat) {}

  // TODO
  template <class V>
  void dump_line_as_vector() {}

  // buggy
  template <class T>
  void files_merge(const T & fn, 
                   const paracel::str_type & prefix = "result_",
                   bool append_flag = false) {
    auto fname_lst = paracel::expand(fn);
    std::ofstream os;
    if(append_flag) {
      os.open(paracel::todir(output) + prefix + "merge",
              std::ofstream::app);
    } else {
      os.open(paracel::todir(output) + prefix + "merge");
    }
    for(auto & fname : fname_lst) {
      std::ifstream f(fname, std::ios::binary);
      if(!f) {
        throw std::runtime_error("internal error in files_merge: open reading failed.");
      }
      paracel::str_type l;
      while(std::getline(f, l)) {
        os << l << '\n';
      }
      f.close();
    }
    os.close();
  }

  // TODO
  template <class V>
  void paracel_dump_vector(const paracel::list_type<V> & data,
                           const paracel::dict_type<size_t, paracel::str_type> & id_map,
                           const paracel::str_type & filename = "result_",
                           const paracel::str_type & sep = ",",
                           bool append_flag = false,
                           bool merge = false) {}

  template <class V>
  void paracel_dump_vector(const paracel::list_type<V> & data, 
                           const paracel::str_type & filename = "result_",
                           const paracel::str_type & sep = ",",
                           bool append_flag = false,
                           bool merge = false) {
    std::ofstream os;
    if(append_flag) {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()),
              std::ofstream::app);
    } else {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()));
    }
    for(size_t i = 0; i < data.size() - 1; ++i) {
      os << cvt(data[i]) << sep;
    }
    if(data.size() > 0) {
      os << cvt(data.back()) << '\n';
    }
    os.close();
    if(merge && get_worker_id() == 0) {
      paracel_sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }

  void paracel_dump_dict(const paracel::dict_type<paracel::str_type, int> & data,
                         const paracel::str_type & filename = "result_",
                         bool append_flag = false,
                         bool merge = false) {
    std::ofstream os;
    if(append_flag) {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()),
              std::ofstream::app);
    } else {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()));
    }
    for(auto & kv : data) {
      os << kv.first << '\t' << kv.second << '\n';
    }
    os.close();
    if(merge && get_worker_id() == 0) {
      paracel_sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }

  void paracel_dump_dict(const paracel::dict_type<paracel::str_type, double> & data,
                         const paracel::str_type & filename = "result_",
                         bool append_flag = false,
                         bool merge = false) {
    std::ofstream os;
    if(append_flag) {
    os.open(paracel::todir(output)
            + filename
            + std::to_string(worker_comm.get_rank()),
            std::ofstream::app);
    } else {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()));
    }
    for(auto & kv : data) {
      os << kv.first << '\t' << kv.second << '\n';
    }
    os.close();
    if(merge && get_worker_id() == 0) {
      paracel_sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }

  template <class T, class P>
  void paracel_dump_dict(const paracel::dict_type<
                         T, paracel::list_type<P> > & data, 
                         const paracel::str_type & filename = "result_",
                         bool append_flag = false,
                         bool merge = false) {
    std::ofstream os;
    if(append_flag) {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()),
              std::ofstream::app);
    } else {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()));
    }
    for(auto & kv : data) {
      os << kv.first << '\t';
      for(size_t i = 0; i < kv.second.size() - 1; ++i) {
        os << cvt(kv.second[i]) << "|";
      }
      os << cvt(kv.second.back()) << '\n';
    }
    os.close();
    if(merge && get_worker_id() == 0) {
      paracel_sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }

  void paracel_dump_dict(const paracel::dict_type<paracel::str_type, 
                         paracel::list_type<
                         std::pair<paracel::str_type, double> > > & data, 
                         const paracel::str_type & filename = "result_",
                         bool append_flag = false,
                         bool merge = false) {
    std::ofstream os;
    if(append_flag) {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()),
              std::ofstream::app);
    } else {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()));
    }
    for(auto & kv : data) {
      os << kv.first + '\t';
      for(size_t i = 0; i < kv.second.size() - 1; ++i) {
        os << kv.second[i].first << ':'
            << std::to_string(kv.second[i].second) << '|';
      }
      os << kv.second.back().first << ":" 
          << std::to_string(kv.second.back().second) << '\n';
    }
    os.close();
    if(merge && get_worker_id() == 0) {
      paracel_sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }

  void paracel_dump_dict(const paracel::dict_type<paracel::default_id_type, 
                         paracel::list_type<
                         std::pair<paracel::default_id_type, double> > > & data, 
                         const paracel::str_type & filename = "result_",
                         bool append_flag = false,
                         bool merge = false) {
    std::ofstream os;
    if(append_flag) {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()),
              std::ofstream::app);
    } else {
      os.open(paracel::todir(output)
              + filename
              + std::to_string(worker_comm.get_rank()));
    }
    for(auto & kv : data) {
      os << std::to_string(kv.first) + '\t';
      for(size_t i = 0; i < kv.second.size() - 1; ++i) {
        os << std::to_string(kv.second[i].first) << ':'
            << std::to_string(kv.second[i].second) << '|';
      }
      os << std::to_string(kv.second.back().first) << ":" 
          << std::to_string(kv.second.back().second) << '\n';
    }
    os.close();
    if(merge && get_worker_id() == 0) {
      paracel_sync();
      paracel::str_type output_regx = output + filename + "*";
      files_merge(output_regx, filename);
    }
  }

  void paracel_dump_prob_mtx(const paracel::dict_type<paracel::str_type, 
                                                      paracel::list_type<std::pair<paracel::str_type,
                                                                        double> > > & data,
                              const paracel::str_type & filename = "result_",
                              bool append_flag = false,
                              bool merge = false) {
      std::ofstream os;
      if(append_flag) {
        os.open(paracel::todir(output)
                + filename
                + std::to_string(worker_comm.get_rank()),
                std::ofstream::app);
      } else {
        os.open(paracel::todir(output)
                + filename
                + std::to_string(worker_comm.get_rank()));
      }
      for(auto & kv : data) {
        os << kv.first + '\t';
        if(kv.second.size() == 0) { continue; }
        for(size_t i = 0; i < kv.second.size() - 1; ++i) {
          os << kv.second[i].first << ':'
              << std::to_string(kv.second[i].second) << '|';
        }
        os << kv.second.back().first << ':'
            << kv.second.back().second << '\n';
      }
      os.close();
      if(merge && get_worker_id() == 0) {
        paracel_sync();
        // TODO
      }
  }

  template <class T, class F>
  T data_merge(T & data,
               F & func,
               paracel::Comm & local_comm,
               int rank = 0) {
    return local_comm.treereduce(data, func, rank);
  }

  virtual void solve() {}
  //virtual void solve() = 0;

 private:
  
  class parasrv {

    using l_type = paracel::list_type<paracel::kvclt>;
    using dl_type = paracel::list_type<paracel::dict_type<paracel::str_type, paracel::str_type> >; 

   public:
    parasrv(paracel::str_type hosts_dct_str) {
      // init dct_lst
      dct_lst = paracel::get_hostnames_dict(hosts_dct_str);
      // init srv_sz
      srv_sz = dct_lst.size();
      // init kvm
      for(auto & srv : dct_lst) {
        paracel::kvclt kvc(srv["host"], srv["ports"]);
        kvm.push_back(std::move(kvc));
      }
      // init servers
      for(auto i = 0; i < srv_sz; ++i) {
        servers.push_back(i);
      }
      // init hashring
      p_ring = new paracel::ring<int>(servers);
    }

    virtual ~parasrv() {
      delete p_ring;
    }

   public:
    dl_type dct_lst;
    int srv_sz = 1;
    l_type kvm;
    paracel::list_type<int> servers;
    paracel::ring<int> *p_ring;

  }; // nested class parasrv 

 private:
  int stale_cache, clock, total_iters;
  int clock_server = 0;
  paracel::Comm worker_comm;
  paracel::str_type output;
  int nworker = 1;
  int rounds = 1;
  int limit_s = 0;
  bool ssp_switch = false;
  parasrv *ps_obj;
  paracel::dict_type<paracel::default_id_type, paracel::default_id_type> rm;
  paracel::dict_type<paracel::default_id_type, paracel::default_id_type> cm;
  paracel::dict_type<paracel::default_id_type, paracel::default_id_type> dm;
  paracel::dict_type<paracel::default_id_type, paracel::default_id_type> col_dm;
  paracel::dict_type<paracel::str_type, paracel::str_type> keymap;
  paracel::dict_type<paracel::str_type, boost::any> cached_para;
  paracel::update_result update_f;
  int npx = 1, npy = 1;
 
 private:
  template <class V>
  struct min_heap_cmp {
    inline bool operator() (const std::pair<paracel::str_type, V> & l,
                            const std::pair<paracel::str_type, V> & r) {
      return l.second > r.second;
    }
  };
  
  std::string cvt(double v) { return std::to_string(v); }

  std::string cvt(const std::string & s) { return s; }

  std::string cvt(const std::pair<paracel::list_type<double>, double> & pr) {
    std::string r;
    for(size_t i = 0; i < pr.first.size() - 1; ++i) {
      r += std::to_string(pr.first[i]) + ",";
    }
    r += std::to_string(pr.first.back()) + ":" + std::to_string(pr.second);
    return r;
  }

  std::string cvt(const std::tuple<std::string, std::string, double> & tpl) {
    return std::get<0>(tpl) + "," + std::get<1>(tpl) + "," + std::to_string(std::get<2>(tpl));
  }

  std::string cvt(const std::tuple<
                  paracel::default_id_type,
                  paracel::default_id_type,
                  double> & tpl) {
    return std::to_string(std::get<0>(tpl)) + ","
        + std::to_string(std::get<1>(tpl)) + ","
        + std::to_string(std::get<2>(tpl));
  }

  std::string cvt(const std::pair<paracel::default_id_type, double> & pr) {
    return std::to_string(pr.first) + ":" + std::to_string(pr.second);
  }

}; // class paralg

} // namespace paracel

#endif
