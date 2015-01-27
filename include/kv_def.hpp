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

#ifndef FILE_627d980b_ee30_b197_b6b4_3a2ca7b11f1e_HPP
#define FILE_627d980b_ee30_b197_b6b4_3a2ca7b11f1e_HPP

#include "paracel_types.hpp"
#include "kv.hpp"

namespace paracel {
  paracel::kvs<paracel::str_type, int> ssp_tbl;
  paracel::kvs<paracel::str_type, paracel::str_type> tbl_store;
}

#endif
