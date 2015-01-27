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

#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>
#include <eigen3/Eigen/Dense>
#include "proxy.hpp"
#include "paracel_types.hpp"

using std::vector;
using std::string;
using std::unordered_map;

extern "C" {
  extern paracel::update_result local_update_kmeans_clusters;
  extern paracel::update_result local_update_kmeans_groups;
}

vector<vector<double> >
local_update_kmeans_clusters_stl(const vector<vector<double> > & a ,
                                 const vector<vector<double> > & b) {
  size_t kclusters = a.size();
  size_t dims = a[0].size();
  vector<vector<double> > r(kclusters);
  for(size_t k = 0; k < kclusters; ++k) {
    r[k].resize(dims);
  }
  for(size_t k = 0; k < kclusters; ++k) {
    for(size_t dim = 0; dim < dims; ++dim) {
      r[k][dim] = a[k][dim] + b[k][dim];
    }
  }
  return r;
}

unordered_map<int, vector<string> >
local_update_kmeans_groups_stl(const unordered_map<int, vector<string> > & a,
                               const unordered_map<int, vector<string> > & b) {
  unordered_map<int, vector<string> > r(a);
  for(auto & kv : b) {
    r[kv.first].insert(r[kv.first].end(), kv.second.begin(), kv.second.end());
  }
  return r;
}

paracel::update_result local_update_kmeans_clusters = paracel::update_proxy(local_update_kmeans_clusters_stl);
paracel::update_result local_update_kmeans_groups = paracel::update_proxy(local_update_kmeans_groups_stl);
