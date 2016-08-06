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
 * Authors: 
 * Hong Wu <xunzhangthu@gmail.com>
 *
 * simple/ugly impl of c++ MPI interface wrapper, just for paracel usage
 * a complete version is implemented internally at Douban.Inc by Changsheng Jiang
 *
 */
     
#ifndef FILE_92d02472_9aee_fd8f_021c_3b914917cd9f_HPP 
#define FILE_92d02472_9aee_fd8f_021c_3b914917cd9f_HPP

#include <tuple>
#include <mpi.h>
#include <iostream>

#include "paracel_types.hpp"
#include "ext_utility.hpp"

namespace paracel {

class main_env {

public:

  main_env(int argc, char *argv[]) { 
    MPI_Init (&argc, &argv); 
  }
  
  ~main_env() { 
    MPI_Finalize();
  }

}; 

class vrequest {
  
typedef paracel::list_type<MPI_Request> vreq_t; 

public:
  vrequest() {}

  vrequest(const vrequest & r) = delete;

  vrequest(MPI_Request && r) {
    vreq.emplace_back(std::move(r));
  }

  vrequest(vrequest &&r) {
    std::swap(vreq, r.vreq);
  }

  void operator=(vrequest &&r) {
    std::swap(vreq, r.vreq);
  }

  void append(vrequest &r) {
    vreq.insert(vreq.end(), r.vreq.begin(), r.vreq.end());
  }

  void wait() {
    for(auto it = vreq.begin(); it != vreq.end(); ++it) {
      MPI_Wait(&*it, MPI_STATUS_IGNORE);
    }
    //MPI_Waitall(vreq.size(), &vreq[0], MPI_STATUS_IGNORE);
  }

  int size() {
    return (int)vreq.size();
  }

  int test() {
    for(auto it = vreq.begin(); it != vreq.end(); ++it) {
      int f = 0;
      MPI_Test(&*it, &f, MPI_STATUS_IGNORE);
      if(!f) {
        return 0;
      }
    }
    return 1;
  }

  vreq_t vreq;
};

class Comm {

public:

  Comm(MPI_Comm comm = MPI_COMM_WORLD);

  Comm(const Comm &);

  Comm(Comm &&);

  ~Comm();

  Comm& operator=(const Comm &);

  Comm& operator=(Comm &&);

  void init(MPI_Comm comm);
  
  void pt_enqueue(int *sz_pt) {
    sz_pt_lst.push_back(sz_pt);
  }

  void pt_enqueue(size_t *key_pt) {
    key_pt_lst.push_back(key_pt);
  }
  
  void pt_enqueue(paracel::str_type *str_pt) {
    str_pt_lst.push_back(str_pt);
  }
  
  void pt_enqueue(double *pt) {
    db_pt_lst.push_back(pt);
  }

  void pt_enqueue(paracel::list_type<double> *pt) {
    lld_pt_lst.push_back(pt);
  }
  
  inline size_t get_size() const { 
    return m_sz; 
  }
  
  inline size_t get_rank() const { 
    return m_rk; 
  }

  inline MPI_Comm get_comm() const { 
    return m_comm; 
  }

  inline void synchronize() { 
    MPI_Barrier(m_comm); 
  }

  int get_source(MPI_Status & stat) { 
    return stat.MPI_SOURCE; 
  }

  Comm split(int color);
  
  void wait(MPI_Request & req) {
    MPI_Wait(&req, MPI_STATUS_IGNORE);
  }
  
  void wait(vrequest & v_req) {
    v_req.wait();
  }

  // impl of is_comm_builtin send
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  send(const T & data, int dest, int tag) {
    MPI_Datatype dtype = paracel::datatype<T>();
    MPI_Send((void *)&data, 1, dtype, dest, tag, m_comm);
  }
  
  // impl of is_comm_container send
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  send(const T & data, int dest, int tag) {
    int sz = (int)data.size();
    send(sz, dest, tag); // send msg size
    send(data, sz, dest, tag);
  }
  
  // impl cont.
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
  send(const T & data, int sz, int dest, int tag) {
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    MPI_Send((void *)&data[0], sz, dtype, dest, tag, m_comm);
  }
  
  // impl of triple send
  void send(const paracel::triple_type & triple, int dest, int tag) {
    auto f = std::get<0>(triple);
    auto s = std::get<1>(triple);
    auto v = std::get<2>(triple);
    send(f, dest, tag);
    send(s, dest, tag);
    send(v, dest, tag);
  }

  // impl of list of triple send
  void send(const paracel::list_type<paracel::triple_type> & triple_lst, int dest, int tag) {
    int sz = triple_lst.size(); // send container size
    send(sz, dest, tag);
    for(size_t i = 0; i < triple_lst.size(); ++i) {
      send(triple_lst[i], dest, tag);
    }
  }
  
  // impl of dict_type<size_t, int> send
  void send(const paracel::dict_type<size_t, int> & dct, int dest, int tag) {
    int sz = dct.size();
    send(sz, dest, tag);
    for(auto & kv : dct) {
      size_t key = kv.first;
      int val = kv.second;
      send(key, dest, tag);
      send(val, dest, tag);
    }
  }
  
  // impl of list of string send
  // TODO: abstract
  void send(const paracel::list_type<paracel::str_type> & strlst, int dest, int tag) {
    int sz = strlst.size(); // send container size
    send(sz, dest, tag);
    for(size_t i = 0; i < strlst.size(); ++i) {
      send(strlst[i], dest, tag);
    }
  }
  
  // impl of is_comm_builtin isend
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value, vrequest>
  isend(const T & data, int dest, int tag) {
    MPI_Datatype dtype = paracel::datatype<T>();
    MPI_Request req;
    MPI_Isend((void *)&data, 1, dtype, dest, tag, m_comm, &req);
    vrequest v_req(std::move(req));
    return v_req;
  }

  // impl of is_comm_container isend
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value, vrequest>
  isend(const T & data, int dest, int tag) {
    vrequest v_req;
    int *sz = new int(data.size());
    pt_enqueue(sz);
    auto sz_req = isend(*sz, dest, tag); // send msg size
    v_req.append(sz_req);
    auto data_req = isend(data, *sz, dest, tag);
    v_req.append(data_req);
    return v_req;
  }
  
  // impl cont.
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value, vrequest>
  isend(const T & data, int sz, int dest, int tag) {
    MPI_Request req;
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    MPI_Isend((void *)&data[0], sz, dtype, dest, tag, m_comm, &req);
    vrequest v_req(std::move(req));
    return v_req;
  }

  // impl of triple isend
  vrequest isend(const paracel::triple_type & triple, int dest, int tag) {
    // pack triple
    paracel::str_type *str_pt = new paracel::str_type(
    	std::get<0>(triple) + paracel::seperator_inner + 
	std::get<1>(triple) + paracel::seperator_inner + 
	std::to_string(std::get<2>(triple))
	);
	  pt_enqueue(str_pt);
    auto v_req = isend(*str_pt, dest, tag);
    return v_req;
  }

  // impl of dict_type<K, V>: <size_t, int>, <string, double>, <string, vector<double> > supported
  template <class K, class V>
  vrequest isend(const paracel::dict_type<K, V> & dct, int dest, int tag) {
    int *sz = new int(dct.size());
    pt_enqueue(sz);
    vrequest v_req = isend(*sz, dest, tag);
    for(auto & kv : dct) {
      K *key = new K(kv.first);
      V *val = new V(kv.second);
      pt_enqueue(key);
      pt_enqueue(val);
      auto kreq = isend(*key, dest, tag);
      auto vreq = isend(*val, dest, tag);
      v_req.append(kreq);
      v_req.append(vreq);
    }
    return v_req;
  }

  // impl of list of triple isend
  vrequest isend(const paracel::list_type<paracel::triple_type> & triple_lst, int dest, int tag) {
    vrequest v_req;
    int *sz = new int(triple_lst.size());
    pt_enqueue(sz);
    auto sz_req = isend(*sz, dest, tag);
    v_req.append(sz_req);
    
    // pack list of triples
    auto lambda_local_pack = [](const paracel::triple_type & tpl) {
      paracel::str_type r;
      r = std::get<0>(tpl) + paracel::seperator_inner + 
          std::get<1>(tpl) + paracel::seperator_inner + 
          std::to_string(std::get<2>(tpl));
      return r;
    };
    paracel::str_type *str_pt = new paracel::str_type;
    pt_enqueue(str_pt);
    if(triple_lst.size() != 0) {
      if(triple_lst.size() == 1) { 
        *str_pt += lambda_local_pack(triple_lst[0]) + paracel::seperator;
      } else {
        for(size_t i = 0; i < triple_lst.size() - 1; ++i) {
          *str_pt += lambda_local_pack(triple_lst[i]) + paracel::seperator;
        }
        *str_pt += lambda_local_pack(triple_lst[triple_lst.size() - 1]);
    
      }
    }
    auto vreq = isend(*str_pt, dest, tag);
    v_req.append(vreq);
    return v_req;
  }

  // impl of list of compact triple isend
  vrequest isend(const paracel::list_type<paracel::compact_triple_type> & triple_lst, int dest, int tag) {
    vrequest v_req;
    int *sz = new int(triple_lst.size());
    pt_enqueue(sz);
    auto sz_req = isend(*sz, dest, tag);
    v_req.append(sz_req);
    
    // pack list of triples
    auto lambda_local_pack = [](const paracel::compact_triple_type & tpl) {
      paracel::str_type r;
      r = std::to_string(std::get<0>(tpl)) + paracel::seperator_inner + 
          std::to_string(std::get<1>(tpl)) + paracel::seperator_inner + 
          std::to_string(std::get<2>(tpl));
      return r;
    };
    paracel::str_type *str_pt = new paracel::str_type;
    pt_enqueue(str_pt);
    if(triple_lst.size() != 0) {
      if(triple_lst.size() == 1) { 
        *str_pt += lambda_local_pack(triple_lst[0]) + paracel::seperator;
      } else {
        for(size_t i = 0; i < triple_lst.size() - 1; ++i) {
          *str_pt += lambda_local_pack(triple_lst[i]) + paracel::seperator;
        }
        *str_pt += lambda_local_pack(triple_lst[triple_lst.size() - 1]);
      }
    }
    auto vreq = isend(*str_pt, dest, tag);
    v_req.append(vreq);
    return v_req;
  }

  // impl of list of string isend
  // TODO: abstract 
  vrequest isend(const paracel::list_type<paracel::str_type> & strlst, int dest, int tag) {
    int *sz = new int(strlst.size()); // send container size
    pt_enqueue(sz);
    vrequest v_req = isend(*sz, dest, tag);
    paracel::str_type *str_pt = new paracel::str_type;
    pt_enqueue(str_pt);
    if(strlst.size() != 0) {
      if(strlst.size() == 1) {
        *str_pt += strlst[0];
      } else {
        for(size_t i = 0; i < strlst.size() - 1; ++i) {
          *str_pt += strlst[i] + paracel::seperator;
        }
        *str_pt += strlst[strlst.size() - 1];
      }
    }
    auto vreq = isend(*str_pt, dest, tag);
    v_req.append(vreq);
    return v_req;
  }

  // impl of is_comm_builtin recv
  // design tip: 
  //   if return recv data, no template var in parameter
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value, MPI_Status>
  recv(T & data, int src, int tag) {
    MPI_Status stat;
    MPI_Datatype dtype = paracel::datatype<T>();
    MPI_Recv(&data, 1, dtype, src, tag, m_comm, &stat);
    return stat;
  }
  
  // impl of is_comm_container recv
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value, MPI_Status>
  recv(T & data, int src, int tag) {
    int sz;
    recv(sz, src, tag); // get msg size
    if(sz) {
      data.resize(sz);
    }
    return recv(data, sz, src, tag);
  }

  // impl cont. 
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value, MPI_Status>
  recv(T & data, int sz, int src, int tag) {
    MPI_Status stat;
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    MPI_Recv((void *)&data[0], sz, dtype, src, tag, m_comm, &stat);
    return stat;
  }

  // impl of triple recv
  MPI_Status recv(paracel::triple_type & triple, int src, int tag) {
    paracel::str_type triple_str;
    MPI_Status stat = recv(triple_str, src, tag);
    // unpack triple
    auto sl = paracel::str_split_by_word(triple_str, paracel::seperator_inner);
    std::get<0>(triple) = sl[0];
    std::get<1>(triple) = sl[1];
    std::get<2>(triple) = std::stod(sl[2]);
    return stat;
  }
  
  // impl of dict_type<K, V>: <size_t, int>, <string, double>, <string, vector<double> > supported
  template <class K, class V>
  MPI_Status recv(paracel::dict_type<K, V> & dct, int src, int tag) {
    int sz;
    K tmp_key;
    V tmp_val;
    MPI_Status stat{};
    recv(sz, src, tag);
    for(int i = 0; i < sz; ++i) {
      recv(tmp_key, src, tag);
      stat = recv(tmp_val, src, tag);
      dct[tmp_key] = tmp_val;
    }
    return stat;
  }

  // impl of list of triple recv
  MPI_Status recv(paracel::list_type<paracel::triple_type> & triple_lst, int src, int tag) {
    int sz;
    recv(sz, src, tag);
    if(sz) {
      triple_lst.resize(sz);
    }
    paracel::str_type triple_lst_str;
    MPI_Status stat = recv(triple_lst_str, src, tag);
    // unpack list of triples
    auto tpl_lst = paracel::str_split_by_word(triple_lst_str, paracel::seperator);
    for(size_t i = 0; i < tpl_lst.size(); ++i) {
      auto tpl = paracel::str_split_by_word(tpl_lst[i], paracel::seperator_inner);
      std::get<0>(triple_lst[i]) = tpl[0];
      std::get<1>(triple_lst[i]) = tpl[1];
      std::get<2>(triple_lst[i]) = std::stod(tpl[2]);
    }
    return stat;
  }

  // impl of list of compact triple recv
  MPI_Status recv(paracel::list_type<paracel::compact_triple_type> & triple_lst, int src, int tag) {
    int sz;
    recv(sz, src, tag);
    if(sz) { triple_lst.resize(sz); }
    paracel::str_type triple_lst_str;
    MPI_Status stat = recv(triple_lst_str, src, tag);
    auto tpl_lst = paracel::str_split_by_word(triple_lst_str, paracel::seperator);
    for(size_t i = 0; i < tpl_lst.size(); ++i) {
      auto tpl = paracel::str_split_by_word(tpl_lst[i], paracel::seperator_inner);
      std::get<0>(triple_lst[i]) = std::stoull(tpl[0]);
      std::get<1>(triple_lst[i]) = std::stoull(tpl[1]);
      std::get<2>(triple_lst[i]) = std::stod(tpl[2]);
    }
    return stat;
  }
  
  // impl of list of string recv
  // TODO: abstract 
  MPI_Status recv(paracel::list_type<paracel::str_type> & strlst, int src, int tag) {
    int sz;
    recv(sz, src, tag);
    if(sz) {
      strlst.resize(sz);
    }
    paracel::str_type str_lst_str;
    MPI_Status stat = recv(str_lst_str, src, tag);
    auto str_lst = paracel::str_split_by_word(str_lst_str, paracel::seperator);
    for(size_t i = 0; i < str_lst.size(); ++i) {
      strlst[i] = str_lst[i]; 
    }
    return stat;
  }

  // impl of sendrecv
  template <class T>
  void sendrecv(const T & sdata, T & rdata, int sto, int stag, int rfrom, int rtag) {
    vrequest v_req = isend(sdata, sto, stag);
    recv(rdata, rfrom, rtag);
    wait(v_req);
    /*
    send(sdata, sto, stag);
    recv(rdata, rfrom, rtag);
    */ 
  }

  // impl of is_comm_builtin bcast
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  bcast(T & data, int master) {
    MPI_Datatype dtype = paracel::datatype<T>();
    MPI_Bcast((void *)&data, 1, dtype, master, m_comm);
  }

  // impl of is_comm_container bcast
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  bcast(T & data, int master) {
    int sz = (int)data.size();
    bcast(data, sz, master);
  }

  // impl cont.
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
  bcast(T & data, int sz, int master) {
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    MPI_Bcast((void *)&data[0], sz, dtype, master, m_comm);
  }

  // impl of bcastring
  template <class T, class F>
  void bcastring(const T & data, F & func) {
    func(data);
    if(m_sz == 1) return;
    for(int i = 1; i < m_sz; ++i) {
      int f = (m_rk + i) % m_sz;
      int t = (m_rk + m_sz - i) % m_sz;
      T rbuf;
      sendrecv(data, rbuf, t, 2014, f, 2014);
      func(rbuf);
    }
  }

  // impl if is_comm_builtin alltoall
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_builtin<typename T::value_type>::value>
  alltoall(const T & sbuf, T & rbuf) {
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    MPI_Alltoall((void *)&sbuf[0], 1, dtype, (void *)&rbuf[0], 1, dtype, m_comm);
  }

  // impl of is_comm_container alltoall
  // impl with sendrecv because MPI_Alltoallv is really awful
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value && paracel::is_comm_container<typename T::value_type>::value>
  alltoall(const T & sbuf, T & rbuf) {
    if(sbuf.size()) {
      rbuf.resize(sbuf.size());
    }  
    rbuf[m_rk] = sbuf[m_rk];
    for(int i = 1; i < m_sz; ++i) {
      int f = (m_rk + i) % m_sz;
      int t = (m_rk + m_sz - i) % m_sz;
      auto tmpr = rbuf[0]; 
      tmpr.clear();
      sendrecv(sbuf[t], tmpr, t, 2014, f, 2014);
      rbuf[t].insert(rbuf[t].end(), tmpr.begin(), tmpr.end());
    }
  }
  
  // impl of triple alltoall
  // TODO: abstract
  void alltoall(const paracel::list_type<paracel::list_type<paracel::triple_type> > & sbuf, 
                paracel::list_type<paracel::list_type<paracel::triple_type> > & rbuf) {
    if(sbuf.size()) {
      rbuf.resize(sbuf.size());
    }
    rbuf[m_rk] = sbuf[m_rk];
    for(int i = 1; i < m_sz; ++i) {
      int f = (m_rk + i) % m_sz;
      int t = (m_rk + m_sz - i) % m_sz;
      paracel::list_type<paracel::triple_type> tmpr;
      sendrecv(sbuf[t], tmpr, t, 2014, f, 2014);
      rbuf[t].insert(rbuf[t].end(), tmpr.begin(), tmpr.end());
    }
  }

  // impl of compact triple alltoall
  void alltoall(const paracel::list_type<paracel::list_type<paracel::compact_triple_type> > & sbuf,
                paracel::list_type<paracel::list_type<paracel::compact_triple_type> > & rbuf) {
    if(sbuf.size()) { rbuf.resize(sbuf.size()); }
    rbuf[m_rk] = sbuf[m_rk];
    for(int i = 1; i < m_sz; ++i) {
      int f = (m_rk + i) % m_sz;
      int t = (m_rk + m_sz - i) % m_sz;
      paracel::list_type<paracel::compact_triple_type> tmpr;
      sendrecv(sbuf[t], tmpr, t, 2014, f, 2014);
      rbuf[t].insert(rbuf[t].end(), tmpr.begin(), tmpr.end());
    }
  }

  // impl of alltoallring
  template <class T, class F>
  void alltoallring(const T & sbuf, T & rbuf, F & func) {
    rbuf.resize(sbuf.size());
    func(sbuf[m_rk]);
    for(int i = 1; i < m_sz; ++i) {
      int f = (m_rk + i) % m_sz;
      int t = (m_rk + m_sz - i) % m_sz;
      auto tmpr = rbuf[0]; 
      tmpr.clear();
      sendrecv(sbuf[t], tmpr, t, 2014, f, 2014);
      func(tmpr);
    }
  }

  // impl of is_comm_builtin allreduce
  // TODO: abstract MPI_SUM 
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<T>::value>
  allreduce(T & data) {
    MPI_Datatype dtype = paracel::datatype<T>();
    T tmp;
    MPI_Allreduce((void *)&data, (void *)&tmp, 1, dtype, MPI_SUM, m_comm);
    data = tmp;
  }

  // impl of is_comm_container allreduce
  // TODO: abstract MPI_SUM 
  template <class T>
  paracel::Enable_if<paracel::is_comm_container<T>::value>
  allreduce(T & data) {
    int sz = (int)data.size();
    allreduce(data, sz);
  }

  // impl cont. 
  template <class T>
  paracel::Enable_if<paracel::is_comm_builtin<typename T::value_type>::value>
  allreduce(T & data, int sz) {
    MPI_Datatype dtype = paracel::container_inner_datatype<T>();
    T tmp;
    tmp.resize(data.size());
    MPI_Allreduce((void *)&data[0], (void *)&tmp[0], sz, dtype, MPI_SUM, m_comm);
    for(size_t i = 0; i < data.size(); ++i) {
      data[i] = tmp[i];
    } 
  }

  /* 
   * warning: SPMD interface, to be invoked by all workers(memory problem!)
   * tree reduce to specified rank
   *
   * void func(const vector<double> & recvbuf, 
   *          vector<double> & sendbuf) {
   *   for(size_t i = 0; i < recvbuf.size(); ++i) {
   *     sendbuf[i] += recvbuf[i];
   *   }
   * }
   *
   * */
  template <class T, class F>
  T treereduce(T & data, F & func, int rank = 0) {
    int rk = m_rk, num = m_sz;
    int is_odd = 0, depth = 1;
    while(num > 1) {
      if(rk < num) {
        is_odd = num % 2;
        // odd send to (odd - 1)
        if(rk % 2 != 0) {
          isend(data, (rk - 1) * depth, 2014);
          rk *= num;
          break; // one procs only send once
        } else {
          if(rk != (num - 1)) {
            T recvbuf;
            recv(recvbuf, (rk + 1) * depth, 2014);
            func(recvbuf, data);
          }
          rk /= 2;
        }
        depth *= 2;
      }
      num = num / 2 + is_odd;
    }
    T r{};
    // last reduce at rk 0
    if(m_rk == 0) {
      r = data;
    }
    // return specified rank
    if(rank != 0) {
      if(m_rk == 0) {
        isend(data, rank, 2014);
      }
      if(m_rk == rank) {
        recv(r, 0, 2014);
      }
    }
    return r;
  }

private:
  MPI_Comm m_comm;
  int m_rk, m_sz;
  paracel::list_type<int *> sz_pt_lst;
  paracel::list_type<double *> db_pt_lst;
  paracel::list_type<size_t *> key_pt_lst;
  paracel::list_type<paracel::str_type *> str_pt_lst;
  paracel::list_type<paracel::list_type<double> *> lld_pt_lst;
}; // class Comm

} // namespace paracel

#endif
