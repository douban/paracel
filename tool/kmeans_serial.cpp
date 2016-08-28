#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <stdexcept>

#include <gflags/gflags.h>

#include "ps.hpp"
#include "load.hpp"
#include "utils.hpp"

namespace paracel {
namespace alg {

class kmeans {
 public:
  kmeans(Comm comm,
         std::string _input,
         std::string _output,
         int k,
         int _rounds = 1,
         int limit_s = 0) : 
      input(_input),
      rounds(_rounds),
      kclusters(k) {
    pt = new paralg(comm, _output, rounds);
    init();
  }

  ~kmeans() {
    delete pt;
    delete clusters_ptr;
  }

  void init() {
    // load data into blk_dmtx
    auto local_parser = [] (const std::string & line) {
      auto r = paracel::str_split(line, '\t');
      auto vec = paracel::str_split(r[1], ',');
      r.resize(vec.size() + 1);
      std::copy(vec.begin(), vec.end(), r.begin() + 1);
      return r;
    };
    auto f_parser = paracel::gen_parser(local_parser);
    pt->paracel_load_as_matrix(blk_dmtx, row_map, input, f_parser);
    //std::cout << "check data" << blk_dmtx.row(0)[0] << std::endl;
    
    // init clusters
    clusters_ptr = new Eigen::MatrixXd(kclusters, blk_dmtx.cols());
    std::vector<size_t> indxs;
    for(int i = 0; i < blk_dmtx.rows(); ++i) {
      indxs.push_back(i);
    }
    //std::random_shuffle(indxs.begin(), indxs.end());
    indxs.resize(kclusters);
    for(int i = 0; i < kclusters; ++i) {
      clusters_ptr->row(i) = blk_dmtx.row(indxs[i]);
    }
  }

  void learning() {
    std::unordered_map<size_t, int> pnt_owner; // matrix_indx -> cluster_indx
    for(int rd = 0; rd < rounds; ++rd) {
      pnt_owner.clear();
      for(size_t i = 0; i < (size_t)blk_dmtx.rows(); ++i) {
        Eigen::MatrixXd::Index indx;
        (clusters_ptr->rowwise() - blk_dmtx.row(i)).rowwise().squaredNorm().minCoeff(&indx);
        pnt_owner[i] = indx;
      }
      std::vector<size_t> cluster_cnt_map(kclusters, 0);
      Eigen::MatrixXd clusters_mtx_tmp = Eigen::MatrixXd::Zero(kclusters, blk_dmtx.cols());
      for(auto & kv : pnt_owner) {
        cluster_cnt_map[kv.second] += 1;
        clusters_mtx_tmp.row(kv.second) += blk_dmtx.row(kv.first);
      }
      for(int k = 0; k < kclusters; ++k) {
        clusters_ptr->row(k) = clusters_mtx_tmp.row(k) * (1. / cluster_cnt_map[k]);
      }
    } // rounds
    for(auto & kv : pnt_owner) {
      groups[kv.second].push_back(row_map[kv.first]);
    }
  }

  void solve() {
    learning();
  }

  void dump_result() {
    pt->paracel_dump_dict(groups, "kmeans_");
    std::unordered_map<int, std::vector<double> > clusters_tmp;
    for(int i = 0; i < kclusters; ++i) {
      clusters_tmp[i] = paracel::evec2vec(clusters_ptr->row(i));
    }
    pt->paracel_dump_dict(clusters_tmp, "centers_");
  }

 private:
  std::string input;
  int rounds;
  int kclusters;
  paralg *pt;

  Eigen::MatrixXd blk_dmtx;
  std::unordered_map<paracel::default_id_type, std::string> row_map; // matrix_indx -> id
  Eigen::MatrixXd *clusters_ptr;
  std::unordered_map<int, std::vector<std::string> > groups; // cluster_indx -> [ids]
}; // class kmeans

} // namespace alg
} // namespace paracel

DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  google::SetUsageMessage("[options]\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  paracel::json_parser jp(FLAGS_cfg_file);
  std::string input, output;
  int ktop, rounds;
  try {
    input = jp.check_parse<std::string>("input");
    output = jp.parse<std::string>("output");
    ktop = jp.parse<int>("kclusters");
    rounds = jp.parse<int>("rounds");
  } catch (const std::invalid_argument & e) {
    std::cerr << e.what();
    return 1;
  }
  paracel::alg::kmeans solver(comm, input, output, ktop, rounds);
  solver.solve();
  solver.dump_result();

  return 0;
}
