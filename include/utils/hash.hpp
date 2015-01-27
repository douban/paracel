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
 * Authors
 *   Changsheng Jiang <jiangzuoyan@gmail.com>
 *   Hong Wu <xunzhangthu@gmail.com>
 *
 */

#ifndef FILE_45bb1820_07f5_4c07_8163_35d6870a4475_HPP
#define FILE_45bb1820_07f5_4c07_8163_35d6870a4475_HPP

#include <sys/types.h>
#include <endian.h>

#include <cstdint>
#include <utility>
#include <functional>
#include <vector>
#include <string>
#include <sstream>

namespace paracel {
namespace utils {

template <class T>
struct hash {
  size_t operator()(const T &t) const {
    return std::hash<T>()(t);
  }
};

struct meta_hash {
  template <class T>
  size_t operator()(const T &t) const {
    return utils::hash<T>()(t);
  }
};

inline uint64_t hash_value_combine(const uint64_t x, const uint64_t y) {
  const uint64_t kMul = 0x9ddfea08eb382d69ULL;
  uint64_t a = (x ^ y) * kMul;
  a ^= (a >> 47);
  uint64_t b = (y ^ a) * kMul;
  b ^= (b >> 47);
  b *= kMul;
  return b;
}

/**
 * Compute hash of v with seed.
 *
 */
template <class T>
inline size_t hash_combine(size_t seed, const T& v) {
  hash<T> hh;
  auto h = hh(v);
  return hash_value_combine(h, seed);
}

template <class L>
struct hash< std::vector<L> > {
  size_t operator()(const std::vector<L> & vec) const {
    std::string buff("vechash");
    for(auto & item : vec) {
      std::ostringstream cvt;
      cvt << item;
      buff.append(cvt.str());
    }
    hash<std::string> hl;
    return hl(buff);
  }
};

template <class L, class R>
struct hash< std::pair<L, R> > {
  size_t operator()(const std::pair<L, R> &n) const {
    hash<L> hl;
    return hash_combine(hl(n.first), n.second);
  }
};

} // namespace utils
} // namespace paracel 

#endif
