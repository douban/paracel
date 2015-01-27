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

#ifndef FILE_275e6247_9a21_93f3_3d3c_51a539d1c8d6_HPP
#define FILE_275e6247_9a21_93f3_3d3c_51a539d1c8d6_HPP

#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <functional>

#include "paracel_types.hpp"

namespace paracel {

class partition {

 public:
  partition(paracel::list_type<paracel::str_type> namelst_in,
            int np_in, paracel::str_type pattern_in)
      : namelst(namelst_in), np(np_in), pattern(pattern_in) {}

  // for test usage
  void file_partition(const paracel::str_type & fname,
                      paracel::list_type<long> & ss,
                      paracel::list_type<long> & ee) {
    std::ifstream f(fname, std::ios::ate);
    long sz = f.tellg();
    f.close();
    int nbk = np;
    long bk_sz = sz / static_cast<long>(nbk);
    for(int i = 0; i < nbk; ++i) {
      long s = i * bk_sz;
      ss.push_back(s);
      if(i == nbk - 1) { 
        long e = sz;
        ee.push_back(e);
      } else {
        long e = (i + 1) * bk_sz;
        ee.push_back(e);
      }
    }
  }

  void files_partition(int blk_sz = paracel::BLK_SZ) {
    if(pattern == "linesplit" || pattern == "fvec") {
      blk_sz = 1;
    }
    slst.resize(0);
    elst.resize(0);
    displs.resize(0);
    displs.resize(namelst.size() + 1, 0);
    for(size_t i = 0; i < displs.size() - 1; ++i) {
      std::ifstream f(namelst[i], std::ios::ate);
      long tmp = f.tellg();
      f.close();
      displs[i + 1] = displs[i] + tmp;
    }
    long sz = displs.back();
    int nbk = np * blk_sz;
    long bk_sz = sz / static_cast<long>(nbk);
    long s, e;
    for(int i = 0; i < nbk; ++i) {
      s = static_cast<long>(i) * bk_sz;
      if(i == nbk - 1) {
        e = sz;
      } else {
        e = (i + 1) * bk_sz;
      }
      assert(s < e);
      slst.push_back(s);
      elst.push_back(e);
    }
  }

  inline paracel::list_type<long> get_start_list() {
    return slst;
  }

  inline paracel::list_type<long> get_end_list() {
    return elst;
  }

  // for test usage
  paracel::list_type<paracel::str_type>
  file_load_lines_impl(const paracel::str_type & fname, 
                      long st,
                      long en) {
    paracel::list_type<paracel::str_type> lines;
    auto offset = st;
    std::ifstream f(fname);
    if(offset) {
      f.seekg(offset - 1);
      paracel::str_type l;
      std::getline(f, l);
      // add edge offset. if no egde, add 1:'\n'
      offset += l.size();
    }
    while(offset < en) {
      paracel::str_type l;
      std::getline(f, l);
      offset += l.size() + 1;
      lines.push_back(l);
    }
    f.close();
    return lines;
  }

  paracel::list_type<paracel::str_type>
  files_load_lines_impl(long st, long en) {
    paracel::list_type<paracel::str_type> lines;
    // to locate files index to load from
    int fst = 0;
    int fen = 0;
    long offset;
    for(size_t i = 0; i < namelst.size(); ++i) {
      if(st >= displs[i]) {
        fst = i;
      }
      if(en > displs[i + 1]) {
        fen = i + 1;
      }
    }
    assert(fst <= fen);
    bool flag = false;
    // load from files
    for(auto fi = fst; fi < fen + 1; ++fi) {
      if(flag) { 
        offset = 0;
      } else {
        offset = st - displs[fi];
      }
      assert(offset >= 0);
      
      std::ifstream f(namelst[fi]);
      if(offset) {
        f.seekg(offset - 1);
        paracel::str_type l;
        std::getline(f, l);
        offset += l.size();
      }
      if(fi == fen) {
        while(offset + displs[fi] < en) {
          paracel::str_type l;
	        std::getline(f, l);
	        offset += l.size() + 1;
          lines.push_back(l);
        }
      } else {
        flag = true;
        while(1) {
          paracel::str_type l;
	        std::getline(f, l);
	        if(l.size() == 0) {
            break;
          }
          lines.push_back(l);
        }
      }
      f.close();
    } // end of for
    return lines;
  }

  template <class F>
  void files_load_lines_impl(long st, long en, F & func) {
    // to locate files index to load from
    int fst = 0;
    int fen = 0;
    long offset;
    for(size_t i = 0; i < namelst.size(); ++i) {
      if(st >= displs[i]) {
        fst = i;
      }
      if(en > displs[i + 1]) {
        fen = i + 1;
      }
    }
    assert(fst <= fen);
    bool flag = false;
    // load from files
    for(auto fi = fst; fi < fen + 1; ++fi) {
      if(flag) { 
        offset = 0;
      } else {
        offset = st - displs[fi];
      }
      assert(offset >= 0);
      
      std::ifstream f(namelst[fi]);
      if(offset) {
        f.seekg(offset - 1);
        paracel::str_type l;
        std::getline(f, l);
        offset += l.size();
      }
      if(fi == fen) {
        while(offset + displs[fi] < en) {
          paracel::str_type l;
	        std::getline(f, l);
	        offset += l.size() + 1;
          func(l);
        }
      } else {
        flag = true;
        while(1) {
          paracel::str_type l;
	        std::getline(f, l);
	        if(l.size() == 0) {
            break;
          }
          func(l);
        }
      }
      f.close();
    } // end of for
  }

 private:
  paracel::list_type<paracel::str_type> namelst;
  int np;
  paracel::str_type pattern;
  paracel::list_type<long> slst, elst, displs;

}; // class partition

} // namespace paracel

#endif
