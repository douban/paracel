/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone https://github.com/douban/paracel.git
 *
 * Authors: 
 *   Hong Wu <xunzhangthu@gmail.com>
 *   Changsheng Jiang <jiangzuoyan@gmail.com>
 *
 */

#include "utils/comm.hpp"

namespace paracel {

Comm::Comm(MPI_Comm comm) {
  init(comm);
}

Comm::Comm(const Comm &r) {
  init(r.m_comm);
}

Comm::Comm(Comm &&r) {
  m_comm = r.m_comm;
  r.m_comm = MPI_COMM_NULL;
  m_rk = r.m_rk;
  m_sz = r.m_sz;
}

Comm::~Comm() {
  MPI_Comm_free(&m_comm);
  for(size_t i = 0; i < sz_pt_lst.size(); ++i) {
    delete sz_pt_lst[i];
  }
  for(size_t i = 0; i < db_pt_lst.size(); ++i) {
    delete db_pt_lst[i];
  }
  for(size_t i = 0; i < str_pt_lst.size(); ++i) {
    delete str_pt_lst[i];
  }
  for(size_t i = 0; i < key_pt_lst.size(); ++i) {
    delete key_pt_lst[i];
  }
  for(size_t i = 0; i < lld_pt_lst.size(); ++i) {
    delete lld_pt_lst[i];
  }
}

Comm& Comm::operator=(const Comm &r) {
  MPI_Comm_free(&m_comm);
  init(r.m_comm);
  return *this;
}

Comm& Comm::operator=(Comm &&r) {
  MPI_Comm_free(&m_comm);
  m_comm = r.m_comm;
  r.m_comm = MPI_COMM_NULL;
  m_rk = r.m_rk;
  m_sz = r.m_sz;
  return *this;
}

void Comm::init(MPI_Comm comm) {
  MPI_Comm_dup(comm, &m_comm);
  MPI_Comm_rank(m_comm, &m_rk);
  MPI_Comm_size(m_comm, &m_sz);
}

Comm Comm::split(int color) {
  MPI_Comm new_comm;
  int key = m_rk;
  MPI_Comm_split(m_comm, color, key, &new_comm);
  Comm comm(new_comm);
  return comm;
}

} // namespace paracel
