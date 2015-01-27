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

#ifndef FILE_71d45241_99cd_4d2c_cb1a_9d3e9ac6203c_HPP
#define FILE_71d45241_99cd_4d2c_cb1a_9d3e9ac6203c_HPP

#include <iostream>
#include <functional>

#include <boost/variant.hpp>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>

#include "paracel_types.hpp"
#include "graph.hpp"
#include "utils.hpp"
#include "load/scheduler.hpp"
#include "load/partition.hpp"

namespace paracel {

using parser_type = std::function<paracel::list_type<paracel::str_type>(paracel::str_type)>;

using var_ret_type = boost::variant<paracel::list_type<paracel::str_type>, bool>;

typedef Eigen::Triplet<double> eigen_triple;
  
void cheat_to_os_local() {
  paracel::list_type<int> var(100000000);
}
  
template <class T = paracel::str_type>
class loader {

 public:

  loader(T fns, paracel::Comm comm) : filenames(fns), m_comm(comm) {};

  loader(T fns, paracel::Comm comm, 
         paracel::str_type pt) : filenames(fns), m_comm(comm), pattern(pt) {};

  loader(T fns, 
         paracel::Comm comm, 
         parser_type f, 
         paracel::str_type pt) : filenames(fns), m_comm(comm), parserfunc(f), pattern(pt) {};

  loader(T fns, 
         paracel::Comm comm, 
         parser_type f, 
         paracel::str_type pt, 
         bool flag) : filenames(fns), m_comm(comm), parserfunc(f), pattern(pt), mix(flag) {};

  paracel::list_type<paracel::str_type> load() {
    
    paracel::scheduler scheduler(m_comm, pattern, mix);
    auto fname_lst = paracel::expand(filenames);
    
    paracel::partition partition_obj(fname_lst,
                                     m_comm.get_size(),
                                     pattern);
    partition_obj.files_partition();
    auto linelst = scheduler.schedule_load(partition_obj);
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "lines got" << std::endl;
    
    return linelst;
  }

  paracel::list_type<paracel::str_type> fixload() {
    
    paracel::scheduler scheduler(m_comm, pattern, mix);
    auto fname_lst = paracel::expand(filenames);
    
    paracel::partition partition_obj(fname_lst,
                                     m_comm.get_size(),
                                     pattern);
    partition_obj.files_partition();
    // parallel loading lines
    auto linelst = scheduler.structure_load(partition_obj);
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "lines got" << std::endl;
    
    return linelst;
  }

  template <class F>
  void fixload_handle(F & func) {
    paracel::scheduler scheduler(m_comm, pattern, mix);
    auto fname_lst = paracel::expand(filenames);
    paracel::partition partition_obj(fname_lst,
                                     m_comm.get_size(),
                                     pattern);
    partition_obj.files_partition();
    scheduler.structure_load_handle(partition_obj, func);
    if(m_comm.get_rank() == 0) std::cout << "lines parsed" << std::endl;
  }

  // fmap case only
  void create_matrix(paracel::list_type<paracel::str_type> & linelst,
                     Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
                     paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & rm,
                     paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & cm) {
    
    paracel::scheduler scheduler(m_comm, pattern, mix);
    
    paracel::list_type<paracel::list_type<paracel::compact_triple_type> > result;
    scheduler.lines_organize(linelst, 
                             parserfunc, 
                             result);
    linelst.resize(0); linelst.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "slotslst generated" << std::endl;
    
    paracel::list_type<paracel::compact_triple_type> stf, stf_new;
    scheduler.exchange(result, stf);
    result.resize(0); result.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "desirable lines got" << std::endl;
    
    scheduler.index_mapping(stf, stf_new, rm, cm);
    if(m_comm.get_rank() == 0) std::cout << "process 0 index mapping finished" << std::endl;
    
    paracel::list_type<eigen_triple> nonzero_tpls;
    for(auto & tpl : stf_new) {
      nonzero_tpls.push_back(eigen_triple(std::get<0>(tpl), 
                                          std::get<1>(tpl), 
                                          std::get<2>(tpl)));
    }
    blk_mtx.resize(rm.size(), cm.size());
    blk_mtx.setFromTriplets(nonzero_tpls.begin(), 
                            nonzero_tpls.end());
  }

  // simple fmap case, fsmap case
  void create_matrix(paracel::list_type<paracel::str_type> & linelst,
                     Eigen::SparseMatrix<double, Eigen::RowMajor> & blk_mtx,
                     paracel::dict_type<paracel::default_id_type, paracel::str_type> & rm, 
                     paracel::dict_type<paracel::default_id_type, paracel::str_type> & cm) {

    paracel::scheduler scheduler(m_comm, pattern, mix); // TODO
    
    // hash lines into slotslst
    auto result = scheduler.lines_organize(linelst, parserfunc);
    linelst.resize(0); linelst.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "slotslst generated" << std::endl;

    // alltoall exchange
    auto stf = scheduler.exchange(result);
    result.resize(0); result.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "desirable lines got" << std::endl;
    
    // mapping inds to ids, get rmap, cmap, std_new...
    paracel::list_type<paracel::compact_triple_type> stf_new;
    scheduler.index_mapping(stf, stf_new, rm, cm);
    if(m_comm.get_rank() == 0) std::cout << "process 0 index mapping finished" << std::endl;
    
    // create block sparse matrix
    paracel::list_type<eigen_triple> nonzero_tpls;
    for(auto & tpl : stf_new) {
      nonzero_tpls.push_back(eigen_triple(std::get<0>(tpl), 
                                          std::get<1>(tpl), 
                                          std::get<2>(tpl)));
    }
    blk_mtx.resize(rm.size(), cm.size());
    blk_mtx.setFromTriplets(nonzero_tpls.begin(), 
                            nonzero_tpls.end());
  }

  // fvec case, only support row decopposition
  void create_matrix(paracel::list_type<paracel::str_type> & linelst,
                     Eigen::MatrixXd & blk_dense_mtx,
                     paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & rm) {
    int csz = 0;
    paracel::default_id_type indx = 0;
    bool flag = true;
    paracel::list_type<Eigen::VectorXd> mtx_llst;
    for(auto & line : linelst) {
      auto stf = parserfunc(line);
      if(flag) { 
        csz = stf.size() - 1; 
        flag = true; 
      }
      rm[indx] = std::stoull(stf[0]);
      indx += 1;
      Eigen::VectorXd tmp(csz);
      for(int i = 0; i < csz; ++i) {
        tmp[i] = std::stod(stf[i + 1]);
      }
      mtx_llst.push_back(tmp);
    } 
    // create dense block matrix
    blk_dense_mtx.resize(rm.size(), csz);
    for(paracel::default_id_type i = 0; i < rm.size(); ++i) {
      blk_dense_mtx.row(i) = mtx_llst[i];
    }
  }

  // fvec case, only support row decomposition
  void create_matrix(paracel::list_type<paracel::str_type> & linelst,
                     Eigen::MatrixXd & blk_dense_mtx,
                     paracel::dict_type<paracel::default_id_type, paracel::str_type> & rm) {

    int csz = 0;
    paracel::default_id_type indx = 0;
    bool flag = true;
    paracel::list_type<Eigen::VectorXd> mtx_llst;
    for(auto & line : linelst) {
      auto stf = parserfunc(line);
      if(flag) { 
        csz = stf.size() - 1; 
        flag = true; 
      }
      rm[indx] = stf[0];
      indx += 1;
      Eigen::VectorXd tmp(csz);
      for(int i = 0; i < csz; ++i) {
        tmp[i] = std::stod(stf[i + 1]);
      }
      mtx_llst.push_back(tmp);
    } 
    // create dense block matrix
    blk_dense_mtx.resize(rm.size(), csz);
    for(paracel::default_id_type i = 0; i < rm.size(); ++i) {
      blk_dense_mtx.row(i) = mtx_llst[i];
    }
  }

  void create_graph(paracel::list_type<paracel::str_type> & linelst,
                    paracel::digraph<paracel::default_id_type> & grp) {
    
    paracel::scheduler scheduler(m_comm, pattern, mix);
    
    paracel::list_type<paracel::list_type<paracel::compact_triple_type> > result;
    scheduler.lines_organize(linelst, 
                             parserfunc, 
                             result);
    linelst.resize(0); linelst.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << " slotslst generated" << std::endl;
    
    paracel::list_type<paracel::compact_triple_type> stf;
    scheduler.exchange(result, stf);
    result.resize(0); result.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    
    for(auto & tpl : stf) {
      grp.add_edge(std::get<0>(tpl), 
                   std::get<1>(tpl), 
                   std::get<2>(tpl));
    }
    stf.resize(0); stf.shrink_to_fit(); paracel::cheat_to_os();
  }

  void create_graph(paracel::list_type<paracel::str_type> & linelst,
                    paracel::digraph<paracel::str_type> & grp) {
    
    paracel::scheduler scheduler(m_comm, pattern, mix); // TODO
    
    // hash lines into slotslst
    auto result = scheduler.lines_organize(linelst, parserfunc);
    linelst.resize(0); linelst.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "slotslst generated" << std::endl;
    
    // alltoall exchange
    auto stf = scheduler.exchange(result);
    result.resize(0); result.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "desirable lines got" << std::endl;

    paracel::dict_type<paracel::str_type, 
                      paracel::dict_type<paracel::str_type, double> > dct;
    for(auto & tpl : stf) {
      dct[std::get<0>(tpl)][std::get<1>(tpl)] = std::get<2>(tpl);  
    }
    grp.construct_from_dict(dct);
  }
  
  void create_graph(paracel::list_type<paracel::str_type> & linelst,
                    paracel::bigraph<paracel::default_id_type> & grp) {
    
    paracel::scheduler scheduler(m_comm, pattern, mix); // TODO
    
    // hash lines into slotslst
    paracel::list_type<paracel::list_type<paracel::compact_triple_type> > result;
    scheduler.lines_organize(linelst, 
                             parserfunc, 
                             result);
    linelst.resize(0); linelst.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "slotslst generated" << std::endl;
    
    // alltoall exchange
    paracel::list_type<paracel::compact_triple_type> stf;
    scheduler.exchange(result, stf);
    result.resize(0); result.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "desirable lines got" << std::endl;
    for(auto & tpl : stf) {
      grp.add_edge(std::get<0>(tpl), 
                   std::get<1>(tpl), 
                   std::get<2>(tpl));
    }
    stf.resize(0); stf.shrink_to_fit(); paracel::cheat_to_os();
  }

  void create_graph(paracel::list_type<paracel::str_type> & linelst,
                    paracel::bigraph<paracel::str_type> & grp) {
    
    paracel::scheduler scheduler(m_comm, pattern, mix); // TODO
    
    // hash lines into slotslst
    auto result = scheduler.lines_organize(linelst, parserfunc);
    linelst.resize(0); linelst.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "slotslst generated" << std::endl;
    
    // alltoall exchange
    auto stf = scheduler.exchange(result);
    result.resize(0); result.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "desirable lines got" << std::endl;

    paracel::dict_type<paracel::str_type, 
                      paracel::dict_type<paracel::str_type, double> > dct;
    for(auto & tpl : stf) { 
      dct[std::get<0>(tpl)][std::get<1>(tpl)] = std::get<2>(tpl); 
    }
    grp.construct_from_dict(dct);
  }

  void create_graph(paracel::list_type<paracel::str_type> & linelst,
                    paracel::bigraph_continuous & grp,
                    paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & rm,
                    paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & cm) {
    
    paracel::scheduler scheduler(m_comm, pattern, mix); // TODO
    
    // hash lines into slotslst
    paracel::list_type<paracel::list_type<paracel::compact_triple_type> > result;
    scheduler.lines_organize(linelst, 
                             parserfunc, 
                             result);
    linelst.resize(0); linelst.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "slotslst generated" << std::endl;
    
    // alltoall exchange
    paracel::list_type<paracel::compact_triple_type> stf;
    scheduler.exchange(result, stf);
    result.resize(0); result.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();
    if(m_comm.get_rank() == 0) std::cout << "desirable lines got" << std::endl;
    
    paracel::list_type<paracel::compact_triple_type> stf_new;
    scheduler.index_mapping(stf, stf_new, rm, cm);
    stf.resize(0); stf.shrink_to_fit(); paracel::cheat_to_os();
    m_comm.sync();

    for(auto & tpl : stf_new) {
      grp.add_edge(std::get<0>(tpl), 
                   std::get<1>(tpl), 
                   std::get<2>(tpl)); 
    } 
    stf_new.resize(0); stf_new.shrink_to_fit(); paracel::cheat_to_os();
  }

 private:
  T filenames;
  paracel::Comm m_comm;
  parser_type parserfunc;
  paracel::str_type pattern = "fmap";
  bool mix = false;

}; // class loader

} // namespace paracel

#endif
