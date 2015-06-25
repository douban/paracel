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

#ifndef FILE_3abacbd9_27ff_b19b_e2a0_88e092dbc44b_HPP 
#define FILE_3abacbd9_27ff_b19b_e2a0_88e092dbc44b_HPP

#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

#include <thread>
#include <mutex>
#include <functional>

#include "zmq.hpp"
#include "utils.hpp"
#include "packer.hpp"
#include "kv_def.hpp"
#include "proxy.hpp"
#include "paracel_types.hpp"

namespace paracel {

std::mutex mutex;

using update_result = paracel::update_result;

using filter_result = paracel::filter_result;

static paracel::str_type local_parse_port(paracel::str_type && s) {
  auto l = paracel::str_split(std::move(s), ':');
  return std::move(l[2]);
}

static void rep_send(zmq::socket_t & sock, paracel::str_type & val) {
  zmq::message_t req(val.size());
  std::memcpy((void *)req.data(), &val[0], val.size());
  sock.send(req);
}

template <class V>
static void rep_pack_send(zmq::socket_t & sock, V & val) {
  paracel::packer<V> pk(val);
  paracel::str_type r;
  pk.pack(r);
  zmq::message_t req(r.size());
  std::memcpy((void *)req.data(), &r[0], r.size());
  sock.send(req);
}

void kv_filter4pullall(const paracel::dict_type<paracel::str_type, paracel::str_type> & dct, 
                       paracel::dict_type<paracel::str_type, paracel::str_type> & new_dct,
                       filter_result filter_func) {
  for(auto & kv : dct) {
    if(filter_func(kv.first, kv.second)) {
      new_dct[kv.first] = kv.second;
    }
  }
}

void kv_filter4remove(const paracel::dict_type<paracel::str_type, paracel::str_type> & dct,
                      filter_result filter_func) {
  for(auto & kv : dct) { 
    paracel::str_type v;
    auto key = kv.first;
    auto exist = paracel::tbl_store.get(key, v);
    if(!exist) {
      ERROR_ABORT("key does not exist");
    }
    if(filter_func(key, v)) {
      paracel::tbl_store.del(key);
    }
  }
}

std::string kv_update(const paracel::str_type & key,
                      const paracel::str_type & v_or_delta,
                      update_result update_func) {
  paracel::str_type val;
  auto exist = paracel::tbl_store.get(key, val);
  if(!exist) {
    paracel::tbl_store.set(key, v_or_delta);
    return v_or_delta;
  }
  std::string new_val = update_func(val, v_or_delta);
  paracel::tbl_store.set(key, new_val);
  return new_val;
}

std::vector<std::string>
kvs_update(const paracel::list_type<paracel::str_type> & key_lst,
           const paracel::list_type<paracel::str_type> & v_or_delta_lst,
           update_result update_func) {
  std::vector<std::string> new_vals;
  for(size_t i = 0; i < key_lst.size(); ++i) {
    new_vals.push_back(kv_update(key_lst[i], v_or_delta_lst[i], update_func));
  }
  return new_vals;
}

// thread entry for ssp 
void thrd_exec_ssp(zmq::socket_t & sock) {
  
  paracel::packer<> pk;
  paracel::ssp_tbl.set("server_clock", 0);
  
  while(1) {
    
    zmq::message_t s;
    sock.recv(&s);
    auto scrip = paracel::str_type(static_cast<const char *>(s.data()), s.size());
    auto msg = paracel::str_split_by_word(scrip, paracel::seperator);
    auto indicator = pk.unpack(msg[0]);
    //std::cout << indicator << std::endl;
    
    if(indicator == "push_int") {
      auto key = pk.unpack(msg[1]);
      paracel::packer<int> pk_i;
      auto val = pk_i.unpack(msg[2]);
      paracel::ssp_tbl.set(key, val);
      bool result = true;
      rep_pack_send(sock, result);
    }
    if(indicator == "incr_int") {
      auto key = pk.unpack(msg[1]);
      if(paracel::startswith(key, "client_clock_")) {
        if(paracel::ssp_tbl.get(key)) {
          paracel::ssp_tbl.incr(key, 1);
        } else {
          paracel::ssp_tbl.set(key, 1);
        }
        if(paracel::ssp_tbl.get(key) >= paracel::ssp_tbl.get("worker_sz")) {
          paracel::ssp_tbl.incr("server_clock", 1);
          paracel::ssp_tbl.set(key, 0); 
        }
      }
      paracel::packer<int> pk_i;
      int delta = pk_i.unpack(msg[2]);
      paracel::ssp_tbl.incr(key, delta);
      bool result = true;
      rep_pack_send(sock, result);
    }
    if(indicator == "pull_int") {
      auto key = pk.unpack(msg[1]);
      int result = 0;
      auto exist = paracel::ssp_tbl.get(key, result);
      if(!exist) {
        paracel::str_type tmp = "nokey";
        rep_send(sock, tmp);
      }
      rep_pack_send(sock, result);
    }
  
  } // while
}

// thread entry
void thrd_exec(zmq::socket_t & sock) {

  paracel::packer<> pk;
  update_result update_f;
  filter_result pullall_special_f;
  filter_result remove_special_f;

  auto dlopen_update_lambda = [&](const paracel::str_type & fn, const paracel::str_type & fcn) {
    void *handler = dlopen(fn.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE); 
    if(!handler) {
      std::cerr << "Cannot open library in dlopen_update_lambda: " << dlerror() << '\n';
      abort();
    }
    auto local = dlsym(handler, fcn.c_str());
    if(!local) {
      std::cerr << "Cannot load symbol in dlopen_update_lambda: " << dlerror() << '\n';
      dlclose(handler);
      abort();
    }
    update_f = *(std::function<paracel::str_type(paracel::str_type, paracel::str_type)>*) local;
    dlclose(handler);
  };

  auto dlopen_pullall_lambda = [&](const paracel::str_type & fn, const paracel::str_type & fcn) {
    void *handler = dlopen(fn.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
    if(!handler) {
      std::cerr << "Cannot open library in dlopen_pullall_lambda: " << dlerror() << '\n';
      abort();
    }
    auto local = dlsym(handler, fcn.c_str());
    if(!local) {
      std::cerr << "Cannot load symbol in dlopen_pullall_lambda: " << dlerror() << '\n';
      dlclose(handler);
      abort();
    }
    pullall_special_f = *(std::function<bool(paracel::str_type, paracel::str_type)>*) local;
    dlclose(handler);
  };

  auto dlopen_remove_lambda = [&](const paracel::str_type & fn, const paracel::str_type & fcn) {
    void *handler = dlopen(fn.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE); 
    if(!handler) {
      std::cerr << "Cannot open library in dlopen_remove_lambda: " << dlerror() << '\n';
      abort();
    }
    auto local = dlsym(handler, fcn.c_str());
    if(!local) {
      std::cerr << "Cannot load symbol in dlopen_remove_lambda: " << dlerror() << '\n';
      dlclose(handler);
      abort();
    }
    remove_special_f = *(std::function<bool(paracel::str_type, paracel::str_type)>*) local;
    dlclose(handler);
  };

  while(1) {
    zmq::message_t s;
    sock.recv(&s);
    auto scrip = paracel::str_type(static_cast<const char *>(s.data()), s.size());
    auto msg = paracel::str_split_by_word(scrip, paracel::seperator);
    auto indicator = pk.unpack(msg[0]);
    
    if(indicator == "contains") {
      auto key = pk.unpack(msg[1]);
      auto result = paracel::tbl_store.contains(key);
      rep_pack_send(sock, result);
    }
    if(indicator == "pull") {
      auto key = pk.unpack(msg[1]);
      paracel::str_type result;
      auto exist = paracel::tbl_store.get(key, result);
      if(!exist) {
        paracel::str_type tmp = "nokey";
        rep_send(sock, tmp); 
      } else {
        rep_send(sock, result);
      }
    }
    if(indicator == "pull_multi") {
      paracel::packer<paracel::list_type<paracel::str_type> > pk_l; 
      auto key_lst = pk_l.unpack(msg[1]);
      auto result = paracel::tbl_store.get_multi(key_lst);
      rep_pack_send(sock, result);
    }
    if(indicator == "pullall") {
      auto dct = paracel::tbl_store.getall();
      rep_pack_send(sock, dct);
    }
    if(indicator == "pullall_special") {
      if(msg.size() == 3) {
        // open request func
        auto file_name = pk.unpack(msg[1]);
        auto func_name = pk.unpack(msg[2]);
        dlopen_pullall_lambda(file_name, func_name);
      } else {
        // work with registered mode
        if(!pullall_special_f) {
          ERROR_ABORT("you must specify a filter for pullall, otherwise you can just use pullall instead");
        }
        // TODO
      }
      auto dct = paracel::tbl_store.getall();
      paracel::dict_type<paracel::str_type, paracel::str_type> new_dct;
      kv_filter4pullall(dct, 
                        new_dct, 
                        pullall_special_f);
      rep_pack_send(sock, new_dct);
    }
    if(indicator == "register_pullall_special") {
      auto file_name = pk.unpack(msg[1]);
      auto func_name = pk.unpack(msg[2]);
      dlopen_pullall_lambda(file_name, func_name);
      bool result = true; 
      rep_pack_send(sock, result);
    }
    if(indicator == "register_remove_special") {
      auto file_name = pk.unpack(msg[1]);
      auto func_name = pk.unpack(msg[2]);
      dlopen_remove_lambda(file_name, func_name);
      bool result = true; 
      rep_pack_send(sock, result);
    }
    if(indicator == "register_update") {
      auto file_name = pk.unpack(msg[1]);
      auto func_name = pk.unpack(msg[2]);
      dlopen_update_lambda(file_name, func_name);
    }
    if(indicator == "register_bupdate") {
      auto file_name = pk.unpack(msg[1]);
      auto func_name = pk.unpack(msg[2]);
      dlopen_update_lambda(file_name, func_name);
      bool result = true; 
      rep_pack_send(sock, result);
    }
    mutex.lock();
    if(indicator == "push") {
      auto key = pk.unpack(msg[1]);
      paracel::tbl_store.set(key, msg[2]);
      bool result = true;
      rep_pack_send(sock, result);
    }
    if(indicator == "push_multi") {
      paracel::packer<paracel::list_type<paracel::str_type> > pk_l;
      paracel::dict_type<paracel::str_type, paracel::str_type> kv_pairs;
      auto key_lst = pk_l.unpack(msg[1]);
      auto val_lst = pk_l.unpack(msg[2]);
      assert(key_lst.size() == val_lst.size());
      for(int i = 0; i < (int)key_lst.size(); ++i) {
        kv_pairs[key_lst[i]] = val_lst[i];
      }
      paracel::tbl_store.set_multi(kv_pairs);
      bool result = true;
      rep_pack_send(sock, result);
    }
    if(indicator == "update" || indicator == "bupdate") {
      if(msg.size() > 3) {
        if(msg.size() != 5) {
          ERROR_ABORT("invalid invoke in server end");
        }
        // open request func
        auto file_name = pk.unpack(msg[3]);
        auto func_name = pk.unpack(msg[4]);
        dlopen_update_lambda(file_name, func_name);
      } else {
        if(!update_f) {
          dlopen_update_lambda("../local/build/lib/default.so",
                               "default_incr_i");
        }
      }
      auto key = pk.unpack(msg[1]);
      std::string result = kv_update(key, msg[2], update_f);
      rep_send(sock, result);
    }
    if(indicator == "bupdate_multi") {
      if(msg.size() > 3) {
        if(msg.size() != 5) {
          ERROR_ABORT("invalid invoke in server end");
        }
        // open request func
        auto file_name = pk.unpack(msg[3]);
        auto func_name = pk.unpack(msg[4]);
        dlopen_update_lambda(file_name, func_name);
      } else {
        if(!update_f) {
          dlopen_update_lambda("../local/build/lib/default.so",
                               "default_incr_i");
        }
      }
      paracel::packer<paracel::list_type<paracel::str_type> > pk_l;
      auto key_lst = pk_l.unpack(msg[1]);
      auto v_or_delta_lst = pk_l.unpack(msg[2]);
      assert(key_lst.size() == v_or_delta_lst.size());
      auto result = kvs_update(key_lst, v_or_delta_lst, update_f);
      rep_pack_send(sock, result);
    }
    if(indicator == "remove") {
      auto key = pk.unpack(msg[1]);
      auto result = paracel::tbl_store.del(key);
      rep_pack_send(sock, result);
    }
    if(indicator == "remove_special") {
      if(msg.size() == 3) {
        // open request func
        auto file_name = pk.unpack(msg[1]);
        auto func_name = pk.unpack(msg[2]);
        dlopen_remove_lambda(file_name, func_name);
      } else {
        if(!remove_special_f) {
          ERROR_ABORT("you must define a filter to use remove_special, otherwise you can use remove instead");
        }
      }
      auto dct = paracel::tbl_store.getall();
      kv_filter4remove(dct, remove_special_f);
      bool result = true;
      rep_pack_send(sock, result);
    }
    if(indicator == "clear") { 
      paracel::tbl_store.clean();
      bool result = true;
      rep_pack_send(sock, result);
    }
    mutex.unlock();

  } // while

} // thrd_exec

// init_host is the hostname of starter
void init_thrds(const paracel::str_type & init_host, 
                const paracel::str_type & init_port) {

  zmq::context_t context(2);
  zmq::socket_t sock(context, ZMQ_REQ);
  
  paracel::str_type info = "tcp://" + init_host + ":" + init_port;
  sock.connect(info.c_str());

  char hostname[1024], freeport[1024];
  size_t size = sizeof(freeport);
  
  // hostname of servers
  gethostname(hostname, sizeof(hostname));
  paracel::str_type ports = hostname;
  ports += ":";

  // create sock in every thrd
  std::vector<zmq::socket_t *> sock_pt_lst;
  for(int i = 0; i < paracel::threads_num; ++i) {
    zmq::socket_t *tmp;
    tmp = new zmq::socket_t(context, ZMQ_REP);
    sock_pt_lst.push_back(tmp);
    sock_pt_lst.back()->bind("tcp://*:*");
    sock_pt_lst.back()->getsockopt(ZMQ_LAST_ENDPOINT, &freeport, &size);
    if(i == paracel::threads_num - 1) {
      ports += local_parse_port(paracel::str_type(freeport));
    } else {
      ports += local_parse_port(std::move(paracel::str_type(freeport))) + ",";
    }
  }

  zmq::message_t request(ports.size());
  std::memcpy((void *)request.data(), &ports[0], ports.size());
  sock.send(request);

  zmq::message_t reply;
  sock.recv(&reply);

  paracel::list_type<std::thread> threads;
  for(int i = 0; i < paracel::threads_num - 1; ++i) {
    threads.push_back(std::thread(thrd_exec, std::ref(*sock_pt_lst[i])));
  }
  threads.push_back(std::thread(thrd_exec_ssp, std::ref(*sock_pt_lst.back())));

  for(auto & thrd : threads) {
    thrd.join();
  }

  for(int i = 0; i < paracel::threads_num; ++i) {
    delete sock_pt_lst[i];
  }

  zmq_ctx_destroy(context);
} // init_thrds

} // namespace paracel

#endif
