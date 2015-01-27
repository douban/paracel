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

#ifndef FILE_a0d2b4a9_6b3d_70e6_20a8_640ae47d8d42_HPP
#define FILE_a0d2b4a9_6b3d_70e6_20a8_640ae47d8d42_HPP

#include <math.h>

namespace paracel {

void npfactx(const int np, int & nx, int & ny) {
  nx = np;
  ny = 1;
}

void npfacty(const int np, int & nx, int & ny) {
  nx = 1;
  ny = np;
}

void npfact2d(const int np, int & nx, int & ny, bool row_master = true) {
  int upbnd = (int)sqrt(np);
  while(upbnd - 1) {
    if((np % upbnd) == 0) {
      if(row_master) {
        nx = upbnd;
        ny = np / upbnd;
        return;
      } else {
        nx = np / upbnd;
        ny = upbnd;
        return;
      }
    } else {
      upbnd -= 1;
    }
  } // end of while
  if(row_master) { 
    nx = np; ny = 1; 
  } else { 
    nx = 1; ny = np; 
  }
  return;
}

} //namespace paracel

#endif
