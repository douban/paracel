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

#include <mutex>

#include "utils/ext_utility.hpp"
#include "utils/comm.hpp"
#include "load/partition.hpp"
#include "load/scheduler.hpp"

namespace paracel {

void scheduler::dim_init() {
  int np = m_comm.get_size();
  npfactx(np, npx, npy);
  if(pattern == "fsmap") {
    npfact2d(np, npx, npy);
  }
  if(pattern == "smap") {
    npfacty(np, npx, npy);
  }
}

paracel::list_type<paracel::str_type> 
scheduler::schedule_load(partition & partition_obj) {
  paracel::list_type<paracel::str_type> result;
  int rk = m_comm.get_rank();
  int sz = m_comm.get_size();
  int flag = 0;
  int blk_sz = BLK_SZ;
  if(pattern == "fvec" || pattern == "linesplit") {
    blk_sz = 1;
  }
  int cnt_control = blk_sz;
  int cnt = cnt_control - 1;
  int ntasks = sz * cnt_control;
  int bnd = ntasks + sz - 2;
  
  auto slst = partition_obj.get_start_list();
  auto elst = partition_obj.get_end_list();

  if(rk == leader) {
    // load tasks [0, BLK_SZ - 1]
    for(int i = 0; i < blk_sz; ++i) {
      // loading lines
      auto lines = partition_obj.files_load_lines_impl(slst[i], elst[i]);
      result.insert(result.end(), lines.begin(), lines.end());
    }
  }
  if(rk != leader) {
    while(1) {
      if(flag) break;
      if(cnt == ntasks - 1) break;
      m_comm.send(rk, leader, 2014);
      m_comm.recv(cnt, leader, 2014);
      m_comm.recv(flag, leader, 2014);
      if(!flag) {
        // loading lines
        auto lines = partition_obj.files_load_lines_impl(slst[cnt], elst[cnt]);
        result.insert(result.end(), lines.begin(), lines.end());
      }
    } // end of while
  } else {
    while(cnt_control < bnd) {
      cnt_control += 1;
      scheduler_mtx.lock();
      int tmp;
      auto status = m_comm.recv(tmp, any_source, any_tag);
      if(!flag) cnt += 1;
      int src = m_comm.get_source(status);
      m_comm.send(cnt, src, 2014);

      if( (cnt == ntasks - 1) && (!flag) ) {
        m_comm.send(flag, src, 2014);
        flag = 1;
      } else {
        m_comm.send(flag, src, 2014);
      }
      scheduler_mtx.unlock();
    } // end of while
  } // end of if-else
  return result;
}

paracel::list_type<paracel::str_type>
scheduler::structure_load(partition & partition_obj) {
  paracel::list_type<paracel::str_type> result;
  int blk_sz = paracel::BLK_SZ;
  if(pattern == "fvec" || pattern == "linesplit") {
    blk_sz = 1;
  }
  int st = m_comm.get_rank() * blk_sz;
  int en = (m_comm.get_rank() + 1) * blk_sz;
  auto slst = partition_obj.get_start_list();
  auto elst = partition_obj.get_end_list();
  for(int i = st; i < en; ++i) {
    auto lines = partition_obj.files_load_lines_impl(slst[i], elst[i]);
    result.insert(result.end(), lines.begin(), lines.end());
  }
  return result;
}

template <class F>
void scheduler::schedule_load_handle(paracel::partition & partition_obj,
                                     F & func) {
  int rk = m_comm.get_rank();
  int sz = m_comm.get_size();
  int flag = 0;
  int blk_sz = BLK_SZ;
  if(pattern == "fvec" || pattern == "linesplit") {
    blk_sz = 1;
  }
  int cnt_control = blk_sz;
  int cnt = cnt_control - 1;
  int ntasks = sz * cnt_control;
  int bnd = ntasks + sz - 2;
  
  auto slst = partition_obj.get_start_list();
  auto elst = partition_obj.get_end_list();

  if(rk == leader) {
    // load tasks [0, BLK_SZ - 1]
    for(int i = 0; i < blk_sz; ++i) {
      // loading lines
      partition_obj.files_load_lines_impl(slst[i], elst[i], func);
    }
  }
  if(rk != leader) {
    while(1) {
      if(flag) break;
      if(cnt == ntasks - 1) break;
      m_comm.send(rk, leader, 2014);
      m_comm.recv(cnt, leader, 2014);
      m_comm.recv(flag, leader, 2014);
      if(!flag) {
        // loading lines
        partition_obj.files_load_lines_impl(slst[cnt], elst[cnt], func);
      }
    } // end of while
  } else {
    while(cnt_control < bnd) {
      cnt_control += 1;
      scheduler_mtx.lock();
      int tmp;
      auto status = m_comm.recv(tmp, any_source, any_tag);
      if(!flag) cnt += 1;
      int src = m_comm.get_source(status);
      m_comm.send(cnt, src, 2014);

      if( (cnt == ntasks - 1) && (!flag) ) {
        m_comm.send(flag, src, 2014);
        flag = 1;
      } else {
        m_comm.send(flag, src, 2014);
      }
      scheduler_mtx.unlock();
    } // end of while
  } // end of if-else
}

} // namespace paracel
