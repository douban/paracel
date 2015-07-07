/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Distributed under the BSD License. Check out the LICENSE file for full text.
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone https://gihub.com/douban/paracel.git
 *
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#ifndef FILE_8199e926_217a_8205_0832_1df88970b15d_HPP
#define FILE_8199e926_217a_8205_0832_1df88970b15d_HPP

#include <time.h>

#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <functional>

#include "partition.hpp"
#include "paracel_types.hpp"
#include "utils/comm.hpp"
#include "utils/decomp.hpp"
#include "utils/ext_utility.hpp"

namespace paracel {

std::mutex scheduler_mtx;

typedef paracel::list_type<paracel::triple_type> listriple_type;
typedef paracel::list_type<paracel::list_type<paracel::triple_type> > listlistriple_type;

auto default_parser = [] (const paracel::str_type & a) { 
  return a; 
};

class scheduler {

public:
  scheduler(paracel::Comm comm) : m_comm(comm) { 
    dim_init(); 
  }

  scheduler(paracel::Comm comm, 
            std::string pt, 
            bool flag) : mix(flag), pattern(pt), m_comm(comm) { 
    dim_init(); 
  }
  
  void dim_init();
   
  paracel::list_type<paracel::str_type> schedule_load(paracel::partition &);
  
  paracel::list_type<paracel::str_type> structure_load(paracel::partition &);

  template <class F>
  void schedule_load_handle(paracel::partition &, F &);

  template <class F>
  void structure_load_handle(paracel::partition & partition_obj,
                                        F & func) {
    int blk_sz = paracel::BLK_SZ;
    if(pattern == "fvec" || pattern == "linesplit") {
      blk_sz = 1;
    }
    int st = m_comm.get_rank() * blk_sz;
    int en = (m_comm.get_rank() + 1) * blk_sz;
    auto slst = partition_obj.get_start_list();
    auto elst = partition_obj.get_end_list();
    for(int i = st; i < en; ++i) {
      partition_obj.files_load_lines_impl(slst[i], elst[i], func);
    }
  }

  inline size_t h(paracel::default_id_type i, paracel::default_id_type j, int nx, int ny) {
    paracel::hash_type<paracel::default_id_type> hf1, hf2;
    return (hf1(i) % nx) * ny + hf2(j) % ny;
  }

  template <class A, class B>
  inline size_t h(A & i, B & j, int & nx, int & ny) { 
    paracel::hash_type<A> hf1; 
    paracel::hash_type<B> hf2;
    size_t r = (hf1(i) % nx) * ny + hf2(j) % ny;
    return r;
  }

  template <class A, class B>
  inline size_t select(A & i, B & j) const {
    paracel::hash_type<A> hf1;
    paracel::hash_type<B> hf2;
    return (hf1(i) % npx) * npy + hf2(j) % npy;
  }

  template <class A>
  inline size_t select(A & i) const {
    paracel::hash_type<A> hf;
    return hf(i) % npx;
  }

  template <class F>
  void lines_organize(const paracel::list_type<paracel::str_type> & lines,
                      F && parser_func,
                      paracel::list_type<
                        paracel::list_type<
                          paracel::compact_triple_type> > & line_slot_lst) {
    line_slot_lst.resize(m_comm.get_size());
    paracel::str_type delimiter("[:| ]*");
    for(auto & line : lines) {
      auto stf = parser_func(line);
      if(stf.size() == 2) {
        auto tmp = paracel::str_split(stf[1], delimiter);
        if(tmp.size() == 1) {
          paracel::compact_triple_type tpl(std::stoull(stf[0]), 
                                           std::stoull(stf[1]), 1.);
          line_slot_lst[h(std::stoull(stf[0]), std::stoull(stf[1]), npx, npy)].push_back(tpl);
        } else {
          paracel::compact_triple_type tpl(std::stoull(stf[0]), 
                                           std::stoull(tmp[0]), 
                                           std::stod(tmp[1]));
          line_slot_lst[h(std::stoull(stf[0]), std::stoull(tmp[0]), npx, npy)].push_back(tpl);
        }
      } else if(mix) {
        for(paracel::default_id_type i = 1; i < stf.size(); ++i) {
          auto item = stf[i];
          auto tmp = paracel::str_split(item, delimiter);
          if(tmp.size() == 1) {
            paracel::compact_triple_type tpl(std::stoull(stf[0]), 
                                             std::stoull(item), 1.);
            line_slot_lst[h(std::stoull(stf[0]), std::stoull(item), npx, npy)].push_back(tpl);
          } else {
            paracel::compact_triple_type tpl(std::stoull(stf[0]), 
                                             std::stoull(tmp[0]), 
                                             std::stod(tmp[1]));
            line_slot_lst[h(std::stoull(stf[0]), std::stoull(tmp[0]), npx, npy)].push_back(tpl);
          }
        }
      } else {
        if(stf.size() != 3) {
          std::cout << line << std::endl;
          throw std::runtime_error("internal error in lines_organize: fmt of input files not supported"); 
        }
        paracel::compact_triple_type tpl(std::stoull(stf[0]), 
                                         std::stoull(stf[1]), 
                                         std::stod(stf[2]));
	      line_slot_lst[h(std::stoull(stf[0]), std::stoull(stf[1]), npx, npy)].push_back(tpl);
      }
    } // for
  }

  template <class F = std::function< paracel::list_type<paracel::str_type>(paracel::str_type) > >
  listlistriple_type 
  lines_organize(const paracel::list_type<paracel::str_type> & lines,
                 F && parser_func = default_parser) {

    listlistriple_type line_slot_lst(m_comm.get_size());
    paracel::str_type delimiter("[:| ]*");
    for(auto & line : lines) { 
      auto stf = parser_func(line);
      if(stf.size() == 2) {
        // bfs or part of fset case
	      // ['a', 'b'] or ['a', 'b:0.2']
	      auto tmp = paracel::str_split(stf[1], delimiter);
	      if(tmp.size() == 1) {
	        paracel::triple_type tpl(stf[0], stf[1], 1.);
	        line_slot_lst[h(stf[0], stf[1], npx, npy)].push_back(tpl);
	      } else {
	        paracel::triple_type tpl(stf[0], tmp[0], std::stod(tmp[1]));
	        line_slot_lst[h(stf[0], tmp[0], npx, npy)].push_back(tpl);
	      }
      } else if(mix) {
        // fset case
	      // ['a', 'b', 'c'] or ['a', 'b|0.2', 'c|0.4']
        // but ['a', '0.2', '0.4'] is not supported here
        for(paracel::default_id_type i = 1; i < stf.size(); ++i) {
	        auto item = stf[i];
	        auto tmp = paracel::str_split(item, delimiter);
	        if(tmp.size() == 1) {
	          paracel::triple_type tpl(stf[0], item, 1.);
	          line_slot_lst[h(stf[0], item, npx, npy)].push_back(tpl);
	        } else {
	          paracel::triple_type tpl(stf[0], tmp[0], std::stod(tmp[1]));
	          line_slot_lst[h(stf[0], tmp[0], npx, npy)].push_back(tpl);
	        }
	      } // end of for
      } else {
	      if(stf.size() != 3) { 
          std::cout << line << std::endl;
          throw std::runtime_error("internal error in lines_organize: fmt of input files not supported"); 
        }
        // fsv case
        paracel::triple_type tpl(stf[0], stf[1], std::stod(stf[2]));
	      line_slot_lst[h(stf[0], stf[1], npx, npy)].push_back(tpl);
      } // end of if
    } // end of for
    return line_slot_lst;
  }

  void exchange(paracel::list_type<paracel::list_type<paracel::compact_triple_type> > & line_slot_lst,
                paracel::list_type<paracel::compact_triple_type> & stf) {
    paracel::list_type<paracel::list_type<paracel::compact_triple_type> > recv_lsl;
    m_comm.alltoall(line_slot_lst, recv_lsl);
    for(auto & lst : recv_lsl) {
      for(auto & tpl : lst) {
        stf.push_back(tpl);
      }
    }
  }

  listriple_type exchange(listlistriple_type & line_slot_lst) {
    listlistriple_type recv_lsl;
    listriple_type stf;
    m_comm.alltoall(line_slot_lst, recv_lsl);
    for(auto & lst : recv_lsl) {
      for(auto & tpl : lst) {
        stf.push_back(tpl);
      }
    }
    return stf;
  }
 
  void index_mapping(const listriple_type & slotslst,
                     paracel::list_type<paracel::compact_triple_type> & stf,
                     paracel::dict_type<paracel::default_id_type, paracel::str_type> & rm,
                     paracel::dict_type<paracel::default_id_type, paracel::str_type> & cm) {
    
    int rk = m_comm.get_rank();
    int rowcolor = rk / npy;
    int colcolor = rk % npy;
    auto col_comm = m_comm.split(colcolor);
    auto row_comm = m_comm.split(rowcolor);

    paracel::list_type<paracel::str_type> rows, cols;
    for(auto & tpl : slotslst) {
      rows.push_back(std::get<0>(tpl));
      cols.push_back(std::get<1>(tpl));
    }
    
    paracel::set_type<paracel::str_type> new_rows, new_cols;
    auto union_func1 = [&new_rows] (paracel::list_type<paracel::str_type> tmp) {
      for(auto & item : tmp) { 
        new_rows.insert(item); 
      }
    };
    auto union_func2 = [&new_cols] (paracel::list_type<paracel::str_type> tmp) {
      for(auto & item : tmp) { 
        new_cols.insert(item); 
      }
    };
    row_comm.bcastring(rows, union_func1);
    col_comm.bcastring(cols, union_func2);
    
    paracel::dict_type<paracel::str_type, 
                      paracel::default_id_type> rev_rm, rev_cm;
    paracel::default_id_type indx = 0;
    for(auto & item : new_rows) {
      rm[indx] = item;
      rev_rm[item] = indx;
      indx += 1;
    }
    indx = 0;
    for(auto & item : new_cols) {
      cm[indx] = item;
      rev_cm[item] = indx;
      indx += 1;
    }

    for(auto & tpl : slotslst) {
      paracel::compact_triple_type tmp;
      std::get<0>(tmp) = rev_rm[std::get<0>(tpl)];
      std::get<1>(tmp) = rev_cm[std::get<1>(tpl)];
      std::get<2>(tmp) = std::get<2>(tpl);
      stf.push_back(tmp);
    }
    rev_rm.clear();
    rev_cm.clear();
    
  } // index_mapping
  
  void index_mapping(const paracel::list_type<paracel::compact_triple_type> & slotslst, 
                     paracel::list_type<paracel::compact_triple_type> & stf,
                     paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & rm,
                     paracel::dict_type<paracel::default_id_type, paracel::default_id_type> & cm) {
    
    int rk = m_comm.get_rank();
    int rowcolor = rk / npy;
    int colcolor = rk % npy;
    auto col_comm = m_comm.split(colcolor);
    auto row_comm = m_comm.split(rowcolor);
    paracel::list_type<int> rows, cols;
    for(auto & tpl : slotslst) {
      rows.push_back(std::get<0>(tpl));
      cols.push_back(std::get<1>(tpl));
    }
    paracel::set_type<int> new_rows, new_cols;
    auto union_func1 = [&new_rows] (paracel::list_type<int> tmp) {
      for(auto & item : tmp) {
        new_rows.insert(item); 
      }
    };
    auto union_func2 = [&new_cols] (paracel::list_type<int> tmp) {
      for(auto & item : tmp) { 
        new_cols.insert(item); 
      }
    };
    row_comm.bcastring(rows, union_func1);
    col_comm.bcastring(cols, union_func2);
    paracel::dict_type<paracel::default_id_type, 
                      paracel::default_id_type> rev_rm, rev_cm;
    paracel::default_id_type indx = 0;
    for(auto & item : new_rows) {
      rm[indx] = item;
      rev_rm[item] = indx;
      indx += 1;
    }
    indx = 0;
    for(auto & item : new_cols) {
      cm[indx] = item;
      rev_cm[item] = indx;
      indx += 1;
    }
    for(auto & tpl : slotslst) {
      paracel::compact_triple_type tmp;
      std::get<0>(tmp) = rev_rm[std::get<0>(tpl)];
      std::get<1>(tmp) = rev_cm[std::get<1>(tpl)];
      std::get<2>(tmp) = std::get<2>(tpl);
      stf.push_back(tmp);
    }
  } // index_mapping
  
private:
  void elect() { 
    auto randint = [] (int l, int u) {
      srand((unsigned)time(NULL));
      return l + rand() % (u - l + 1);
    };
    leader = randint(0, m_comm.get_size() - 1); 
  }

private:
  bool mix;
  paracel::str_type pattern = "fmap"; 
  paracel::Comm m_comm;
  int npx;
  int npy;
  int leader = 0;

}; // class scheduler

} // namespace paracel

#endif
