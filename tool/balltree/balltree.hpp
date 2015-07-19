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

#ifndef FILE_ea618c05_40e1_4878_b628_c33df8bffccd_HPP
#define FILE_ea618c05_40e1_4878_b628_c33df8bffccd_HPP 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#include <queue>
#include <vector>
#include <limits>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <unordered_set>

#include "utils.hpp"

namespace paracel {

struct min_heap_cmp {
  inline bool operator() (const std::pair<long, double >& l,
                          const std::pair<long, double >& r) {
    return l.second > r.second;		
  }
};

struct heap_node {
  heap_node(long id, double v) {
    val = std::pair<long, double>(id, v);
  }
  std::pair<long, double> val;
};

using min_heap = std::priority_queue<std::pair<long, double>, 
                                    std::vector<std::pair<long, double> >, 
                                    min_heap_cmp>;

inline double norm(const std::vector<double> & a) {
  double norm = 0.;
  for(auto & i : a) { 
    norm += i * i; 
  }
  return sqrt(norm);
}
 
inline double euclid_dist(const std::vector<double> & a, 
                          const std::vector<double> & b) {
  double sum = 0.;
  for(int i = 0; i < (int)a.size(); ++i) {
    double s = b[i] - a[i];
    sum += s * s;
  }
  return sqrt(sum);
}

struct query {
 
 public:
  query(std::vector<double> _item) : 
      lambda(std::numeric_limits<double>::lowest()), bm_indx(0), k(0) { 
    item = _item; 
    norm = paracel::norm(item);
    whitelst.clear();
  }

  query(std::vector<double> _item, int _k) : 
      lambda(std::numeric_limits<double>::lowest()), bm_indx(0) { 
    k = _k;
    item = _item; 
    norm = paracel::norm(item);  
    whitelst.clear();
  }

  query(std::unordered_set<long> _blacklst, 
        std::vector<double> _item, 
        int _k) : lambda(std::numeric_limits<double>::lowest()), bm_indx(0) { 
    k = _k;
    item = _item;
    blacklst = _blacklst;
    norm = paracel::norm(item); 
    whitelst.clear();
  }

  query(std::unordered_set<long> _whitelst,
        std::unordered_set<long> _blacklst,
        std::vector<double> _item,
        int _k) : lambda(std::numeric_limits<double>::lowest()), bm_indx(0) {
    k = _k;
    item = _item;
    whitelst = _whitelst;
    blacklst = _blacklst;
    norm = paracel::norm(item);
  }
  int get_k() {
    return k;
  }
  std::vector<double> get_item() {
    return item;
  }
  std::unordered_set<long> get_blacklist() {
    return blacklst;
  }
  std::unordered_set<long> get_whitelist() {
    return whitelst;
  }

 public:
  std::vector<double> item;
  std::unordered_set<long> blacklst;
  std::unordered_set<long> whitelst;
  double lambda;
  double bm_indx;
  double norm;
  int k;
};

struct balltree_node {
 
 public:
  balltree_node(std::vector<long> input_indices) : 
      radius(0.), left(NULL), right(NULL) {
    indices = input_indices;
  }

  balltree_node(double r,
                std::vector<double> _miu,
                std::vector<long> _indices) : 
      radius(r), left(NULL), right(NULL), miu(_miu), indices(_indices) {}

  ~balltree_node() {
    //delete left;
    //delete right;
    indices.resize(0);
  }

 public:
  double radius;
  balltree_node *left;
  balltree_node *right;
  std::vector<double> miu;
  std::vector<long> indices;
}; // struct balltree_node

template <class T> 
struct balltree {

 public:
  balltree(std::vector<std::vector<T> > _items) : 
      ball_limit(20), root(NULL) {
    items = _items;
  } 

  balltree(int limit, std::vector<std::vector<T> > _items) :
      ball_limit(limit), root(NULL) {
    items = _items;
  }

  ~balltree() {
    destroy();
    //delete root;
    items.resize(0);
  }

  void post_order_del(balltree_node *p) {
    if(p == NULL) return;
    post_order_del(p->left);
    post_order_del(p->right);
    delete p;
  }

  void destroy() {
    post_order_del(root);
  }
  
  void build() {
    std::vector<long> indices(items.size());
    for(size_t i = 0; i < items.size(); ++i) {
      indices[i] = i;
    }
    build(indices);
  }
  
  void build(const std::vector<long> & indices) {
    root = build_recsive(indices);
  }

  void dump_vlr(balltree_node *p, std::ofstream & os) {
    if(p == NULL) {
      os << "NULL" << '\n';
    } else {
      // dump radius
      os << p->radius << '\n';
      // dump miu
      for(size_t i = 0; i < p->miu.size() - 1; ++i) {
        os << p->miu[i] << '|';
      }
      os << p->miu.back() << '\n';
      // dump indices
      for(size_t i = 0; i < p->indices.size() - 1; ++i) {
        os << p->indices[i] << '|';
      }
      os << p->indices.back() << '\n';
      dump_vlr(p->left, os);
      dump_vlr(p->right, os);
    }
  }

  void pickle(std::string fn) {
    std::ofstream os;
    os.open(fn, std::ofstream::out);
    dump_vlr(root, os);
    os.close();
  }

  void load_vlr(balltree_node *&p,
                std::ifstream & fin) {
    std::string line;
    std::getline(fin, line);
    if(line == "NULL") {
      return;
    } else {
      std::string miu_line, indices_line;
      std::getline(fin, miu_line);
      std::getline(fin, indices_line);
      double radius = std::stod(line);
      std::vector<double> miu;
      auto tmp = paracel::str_split(miu_line, '|');
      for(auto & v : tmp) miu.push_back(std::stod(v));
      std::vector<long> indices;
      tmp = paracel::str_split(indices_line, '|');
      for(auto & v : tmp) indices.push_back(std::stol(v));
      p = new balltree_node(radius, miu, indices);
      load_vlr(p->left, fin);
      load_vlr(p->right, fin);
    }
  }

  void unpickle(std::string fn) {
    std::ifstream f(fn, std::ios::binary);
    load_vlr(root, f);
    f.close();
  }

  void build_from_file(std::string fn) {
    destroy();
    unpickle(fn);
  }

  // TODO
  void insert(const std::vector<T> & item) {}

  // TODO
  void insert(const balltree_node & node) {}

  void insert(const std::vector<std::vector<T> > & items) {
    for(auto & item : items) {
      insert(item);
    }
  }

  // TODO
  void remove(const std::vector<T> & item) {}

  // TODO
  void remove(const balltree_node & node) {}

  void remove(const std::vector<std::vector<T> > & items) {
    for(auto & item : items) {
      remove(item);
    }
  }
  
 private:
  balltree_node* build_recsive(const std::vector<long> & indices) {
    balltree_node *node = new balltree_node(indices);
    node->miu = cal_mean(indices);
    node->radius = cal_maxr(indices, node->miu);
    if(indices.size() <= (size_t)ball_limit) {
      return node;
    } else {
      std::vector<long> lc_indices, rc_indices;
      split_indices(node->indices, lc_indices, rc_indices);
      if(node->indices.size() != lc_indices.size() && node->indices.size() != rc_indices.size()) {
        node->left = build_recsive(lc_indices);
        node->right = build_recsive(rc_indices);
      }
    }
    return node;
  }

  // calculate center of gravity
  std::vector<T> cal_mean(const std::vector<long> & ids) {
    std::vector<T> center(items[0].size());
    for(auto & id : ids) {
      int i = 0;
      for(auto & dim : items[id]) {
        center[i] += dim;
        i +=1;
      }
    }
    T tmp = 1. / ids.size();
    for(size_t i = 0; i < center.size(); ++i) {
      center[i] *= tmp;
    }
    return center;
  }
  
  /* calculate radius: farthest from center */
  double cal_maxr(const std::vector<long> & ids,
                  const std::vector<double> & miu) {
    std::vector<double> dist_lst;
    for(auto & id : ids) {
      dist_lst.push_back(euclid_dist(items[id], miu));
    }
    return *std::max_element(dist_lst.begin(), dist_lst.end());
  }
  
  std::vector<T> arg_max(const std::vector<T> & basis,
                    const std::vector<long> & cmp_ids) {
    long maxid = 0;
    double t = 0.;
    for(auto & id : cmp_ids) {
      double d = euclid_dist(basis, items[id]);
      auto tmp = d * d;
      if(tmp > t) {
        t = tmp;
        maxid = id;
      }
    }
    return items[maxid];
  }
  
  void partition(const std::vector<long> & ids, 
                 const std::vector<T> & xa,
                 const std::vector<T> & xb,
                 std::vector<long> & lc_ids,
                 std::vector<long> & rc_ids) {		
    lc_ids.resize(0); rc_ids.resize(0);
    for(auto & id : ids) {
      auto d1 = euclid_dist(items[id], xa);
      auto d2 = euclid_dist(items[id], xb);
      if(d1 <= d2) {
        lc_ids.push_back(id);
      } else {
        rc_ids.push_back(id);
      }
    }
  }

  void split_indices(std::vector<long> ids, 
                     std::vector<long> & lc_ids, 
                     std::vector<long> & rc_ids) { 
    srand(time(NULL));
    auto id = rand() % ids.size();
    auto xa = arg_max(items[id], ids);
    auto xb = arg_max(xa, ids);
    partition(ids, xa, xb, lc_ids, rc_ids);
  }

 public:
  int ball_limit;
  balltree_node *root;
  std::vector<std::vector<T> > items;
};

//constexpr auto DIST = euclid_dist;

inline double dot_product(const std::vector<double> & a, const std::vector<double> & b) {
  assert(a.size() == b.size());
  return std::inner_product(a.begin(), a.end(), b.begin(), 0.);
}

int span_cnt = 0;
double max_inner_product(const query & q,
                         const std::vector<double> & miu,
                         double radius) {
  return dot_product(q.item, miu) + q.norm * radius;
}

void linear_search_array(const std::vector<long> & ids,
                         const balltree<double> & stree,
                         query & q,
                         std::vector<std::pair<long, double> > & tmplst) {
  for(auto & id : ids) {
    if(q.blacklst.count(id)) {
      continue;
    }
    if(q.whitelst.size() != 0) {
      if(!q.whitelst.count(id)) {
        continue;
      }
    }
    auto pdt = dot_product(stree.items[id], q.item);
    tmplst.push_back(std::pair<long, double>(id, pdt));
  }
  std::sort(tmplst.begin(), tmplst.end(), 
            [] (std::pair<long, double> a, 
                std::pair<long, double> b) { 
                return a.second > b.second; 
              }
            );
  if((int)tmplst.size() >= q.k) {
    q.lambda = tmplst[tmplst.size() - 1].second;
  }
}

void linear_search_heap(const std::vector<long> & ids,
                        const balltree<double> & stree,
                        query & q,
                        min_heap & tmplst) {
  for(auto & id : ids) {
    if(q.blacklst.count(id)) {
      continue; 
    }
    if(q.whitelst.size() != 0) {
      if(!q.whitelst.count(id)) {
        continue;
      }
    }
    auto pdt = dot_product(stree.items[id], q.item);
    auto node = heap_node(id, pdt);
    tmplst.push(node.val);
    if(tmplst.size() > (size_t)q.k) tmplst.pop();
  }
  assert(tmplst.size() <= (size_t)q.k);
  if(tmplst.size() == (size_t)q.k) {
    q.lambda = tmplst.top().second;
  }
  span_cnt += ids.size();
}

/* array impl */
void balltree_search(const balltree<double> & stree,
                     balltree_node *node,
                     query & q,
                     std::vector<std::pair<long, double> > & tmplst) {
  auto v = max_inner_product(q.item, node->miu, node->radius);
  if(q.lambda < v) {
    // this node has potential
    if((!node->left) && (!node->right)) {
      linear_search_array(node->indices, stree, q, tmplst);
    } else {
      // best depth first traversal
	    auto v_left = max_inner_product(q.item, node->left->miu, node->left->radius);
	    auto v_right = max_inner_product(q.item, node->right->miu, node->right->radius);
      if((q.lambda < v_left) || (q.lambda < v_right)) {
        if(v_left < v_right) {
          balltree_search(stree, node->right, q, tmplst);
          if(q.lambda < v_left) {
            balltree_search(stree, node->left, q, tmplst);
          }
	      } else {
	        balltree_search(stree, node->left, q, tmplst);
	        if(q.lambda < v_right) {
	          balltree_search(stree, node->right, q, tmplst);
          }
	      }
	    }
    }
  } else { 
    // Else the node is pruned from computation
  }
}

/* heap impl */
void balltree_search(const balltree<double> & stree,
                     balltree_node *node,
                     query & q,
                     min_heap & tmplst) {
  auto v = max_inner_product(q.item, node->miu, node->radius);
  if(q.lambda < v) {
    // this node has potential
    if((!node->left) && (!node->right)) {
      linear_search_heap(node->indices, stree, q, tmplst);
    } else {
      // best depth first traversal
	    auto v_left = max_inner_product(q.item, node->left->miu, node->left->radius);
	    auto v_right = max_inner_product(q.item, node->right->miu, node->right->radius);
	    if((q.lambda < v_left) || (q.lambda < v_right)) {
	      if(v_left < v_right) {
	        balltree_search(stree, node->right, q, tmplst);
	        if(q.lambda < v_left) {
	          balltree_search(stree, node->left, q, tmplst);
          }
	      } else {
	        balltree_search(stree, node->left, q, tmplst);
	        if(q.lambda < v_right) {
	          balltree_search(stree, node->right, q, tmplst);
          }
	      }
	    }
    }
  } else { 
    // else the node is pruned from computation
  }
}

// balltree search api
int search(query & q,
           const balltree<double> & stree,
           std::vector<long> & result) {
  std::vector<long> tmp_result;
  result.resize(0);
  min_heap tmplst;
  //span_cnt = 0;
  balltree_search(stree, stree.root, q, tmplst);
  while(!tmplst.empty()) {
    tmp_result.push_back(tmplst.top().first);
    tmplst.pop();
  }
  for(auto & r : tmp_result) {
    if(q.blacklst.count(r)) {
      continue;
    }
    if(q.whitelst.size() != 0) {
      if(!q.whitelst.count(r)) {
        continue;
      }
    }
    result.push_back(r);
  }
  std::reverse(result.begin(), result.end());
  return static_cast<int>(result.size());
}

// brute force pair-wise search
int search(query & q,
           const std::vector<std::vector<double> > & buf,
           std::vector<long> & result) {
  result.resize(0);
  std::vector<std::pair<long, double> > dpt;
  int i = 0;
  for(auto & item_factor : buf) {
    if(q.blacklst.count(i)) {
      i++; 
      continue; 
    }
    if(q.whitelst.size() != 0) {
      if(!q.whitelst.count(i)) {
        i++;
        continue;
      }
    }
    dpt.push_back(
        std::pair<long, double>(i, dot_product(q.item, item_factor))
        );
    i += 1;
  }
  std::sort(dpt.begin(), dpt.end(),
            [] (std::pair<long, double> a, 
                std::pair<long, double> b) {
                  return a.second > b.second;
                }
            );
  for(int i = 0; i < q.k && i < (int)dpt.size(); ++i) {
    result.push_back(dpt[i].first);
  }
  return static_cast<int>(result.size());
}

int search(query & q,
           const balltree<double> & stree,
           std::vector<std::pair<long, double> > & result) {
  std::vector<std::pair<long, double> > tmp_result;
  result.resize(0);
  min_heap tmplst;
  balltree_search(stree, stree.root, q, tmplst);
  while(!tmplst.empty()) {
    tmp_result.push_back(std::make_pair(tmplst.top().first, tmplst.top().second));
    tmplst.pop();
  }
  for(auto & r : tmp_result) {
    if(q.blacklst.count(r.first)) {
      continue;
    }
    if(q.whitelst.size() != 0) {
      if(!q.whitelst.count(r.first)) {
        continue;
      }
    }
    result.push_back(r);
  }
  std::reverse(result.begin(), result.end());
  return static_cast<int>(result.size());
}

int search(query & q,
           const std::vector<std::vector<double> > & buf,
           std::vector<std::pair<long, double> > & result) {
  result.resize(0);
  std::vector<std::pair<long, double> > dpt;
  int i = 0;
  for(auto & item_factor : buf) {
    if(q.blacklst.count(i)) {
      i++; 
      continue; 
    }
    if(q.whitelst.size() != 0) {
      if(!q.whitelst.count(i)) {
        i++;
        continue;
      }
    }
    dpt.push_back(
        std::pair<long, double>(i, dot_product(q.item, item_factor))
        );
    i += 1;
  }
  std::sort(dpt.begin(), dpt.end(),
            [] (std::pair<long, double> a, 
                std::pair<long, double> b) {
                  return a.second > b.second;
                }
            );
  for(int i = 0; i < q.k && i < (int)dpt.size(); ++i) {
    result.push_back(dpt[i]);
  }
  return static_cast<int>(result.size());
}

} // namespace paracel 

#endif
