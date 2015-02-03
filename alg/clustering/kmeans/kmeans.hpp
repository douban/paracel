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

#ifndef FILE_2fae05e7_4f3a_2ac3_dc77_f92f7070193b_HPP
#define FILE_2fae05e7_4f3a_2ac3_dc77_f92f7070193b_HPP

#include <string>
#include <algorithm>
#include <unordered_map>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>

#include "ps.hpp"
#include "load.hpp"

namespace paracel {
namespace alg {

using node_t = paracel::default_id_type;

class kmeans : public paracel::paralg {

 public:
  kmeans(paracel::Comm comm,
         std::string hosts_dct_str,
         std::string _input,
         std::string _output, 
         std::string type,
         int k,
         std::string update_fn,
         std::vector<std::string> update_fcn,
         int _rounds = 1) : 
      paracel::paralg(hosts_dct_str, comm, _output, _rounds, 0, false),
      input(_input),
      rounds(_rounds),
      dtype(type),
      kclusters(k),
      update_file(update_fn),
      update_funcs(update_fcn) {}
 
  virtual ~kmeans() {}

 private:
  // dense matrix
  void local_load_fvec() {}

  // sparse matrix
  void local_load_sim() {}

  // TODO: kmeans++ initialization
  void init() {
    if(dtype == "fvec") {
      // load local dense matrix
      auto local_parser = [] (const std::string & line) {
        auto r = paracel::str_split(line, '\t');
        auto vec = paracel::str_split(r[1], ',');
        r.resize(vec.size() + 1);
        std::copy(vec.begin(), vec.end(), r.begin() + 1);
        return r;
      };
      auto f_parser = paracel::gen_parser(local_parser);
      //paracel_load_as_matrix(blk_dmtx, row_map, input, f_parser);
      paracel_load_as_matrix(blk_dmtx, row_map, input, f_parser, "fvec", true);

      // init clusters
      if(get_worker_id() == 0) {
        std::vector<size_t> indxs;
        for(size_t i = 0; i < (size_t)kclusters; ++i) {
          indxs.push_back(i);
        }
        //std::random_shuffle(indxs.begin(), indxs.end());
        indxs.resize(kclusters);
        std::vector<std::vector<double> > init_clusters;
        for(int i = 0; i < kclusters; ++i) {
          init_clusters.push_back(paracel::evec2vec(blk_dmtx.row(indxs[i])));
        }
        paracel_write("clusters_-1", init_clusters);
      } else { // worker 0
      }
    } else if(dtype == "sim") {
      // TODO: sparsity case 
    }
    sync(); // !
  }

  // TODO: convergence condition
  void learning() {
    std::unordered_map<size_t, int> pnt_owner; // matrix_indx -> cluster_indx 
    paracel_register_bupdate(update_file,
                             update_funcs[0]);

    // main loop
    for(int rd = 0; rd < rounds; ++rd) {
      pnt_owner.clear();
      
      // pull clusters
      clusters = paracel_read<std::vector<std::vector<double> > >("clusters_" + std::to_string(rd - 1));
      
      // convert to eigen
      Eigen::MatrixXd clusters_mtx(kclusters, blk_dmtx.cols());
      for(int k = 0; k < kclusters; ++k) {
        clusters_mtx.row(k) = paracel::vec2evec(clusters[k]);
      }

      // local update
      for(size_t i = 0; i < (size_t)blk_dmtx.rows(); ++i) {
        Eigen::MatrixXd::Index indx;
        (clusters_mtx.rowwise() - blk_dmtx.row(i)).rowwise().squaredNorm().minCoeff(&indx);
        pnt_owner[i] = indx;
      } // sample

      std::vector<size_t> cluster_cnt_map(kclusters, 0);
      Eigen::MatrixXd clusters_mtx_tmp = Eigen::MatrixXd::Zero(kclusters, blk_dmtx.cols());
      for(auto & kv : pnt_owner) {
        cluster_cnt_map[kv.second] += 1;
        clusters_mtx_tmp.row(kv.second) += blk_dmtx.row(kv.first);
      }
      
      // allreduce count for every cluster
      std::vector<size_t> cluster_cnt_map_global(cluster_cnt_map);
      get_comm().allreduce(cluster_cnt_map_global);
      
      // local combine and convert to stl
      for(int k = 0; k < kclusters; ++k) {
        clusters[k] = paracel::evec2vec(clusters_mtx_tmp.row(k) * (1. / cluster_cnt_map_global[k]));
      }

      // update clusters
      paracel_bupdate("clusters_" + std::to_string(rd), clusters);
      sync();
      //paracel_update_default("clusters_" + std::to_string(rd), clusters);
    } // rounds

    // last pull
    clusters = paracel_read<std::vector<std::vector<double> > >("clusters_" + std::to_string(rounds - 1));
      
    // store result into groups
    for(auto & kv : pnt_owner) {
      groups[kv.second].push_back(row_map[kv.first]);
    }

    // allreduce
    paracel_bupdate("kmeans_result", 
                   groups, 
                   update_file, 
                   update_funcs[1]);
    sync();
    groups = paracel_read<std::unordered_map<int, std::vector<std::string> > >("kmeans_result");
  }

 public:
  void solve() {
    init();
    sync();
    learning();
  }

  void dump_result() {
    if(get_worker_id() == 0) {
      paracel_dump_dict(groups, "kmeans_");
      std::unordered_map<int, std::vector<double> > clusters_tmp;
      for(int i = 0; i < kclusters; ++i) {
        clusters_tmp[i] = clusters[i];
      }
      paracel_dump_dict(clusters_tmp, "centers_");
    }
  }

 private:
  std::string input;
  int rounds;
  std::string dtype;
  int kclusters;
  std::string update_file;
  std::vector<std::string> update_funcs;

  //Eigen::SparseMatrix<double, Eigen::RowMajor> blk_smtx;
  Eigen::MatrixXd blk_dmtx;
  std::unordered_map<size_t, std::string> row_map; // matrix_indx -> id
  std::vector<std::vector<double> > clusters;
  std::unordered_map<int, std::vector<std::string> > groups; // cluster_indx -> [ids]
}; // class kmeans

} // namespace alg
} // namespace paracel

#endif
