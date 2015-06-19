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

#include <string>
#include <functional>
#include <google/gflags.h>

#include "ps.hpp"
#include "pregel.hpp"
#include "utils.hpp"
#include "paracel_types.hpp"

namespace paracel {
namespace pregel {

class max_vertex_value : public paracel::pregel::vertex<double, double> {
 
 public:
  max_vertex_value(Comm comm,
                   std::string hosts_dct_str,
                   std::string input,
                   std::string output,
                   paracel::parser_type parser,
                   std::function<double(const std::string &)> init_func) : 
  paracel::pregel::vertex<double, double>(comm, hosts_dct_str, input, output, parser, init_func) {}

  void compute(const std::string & neighbor,
               double wgt,
               const double & val) {
    pt->paracel_bupdate(neighbor,
                        val,
                        "/mfs/user/wuhong/paracel/local/lib/libmvv_update.so",
                        "max_updater");
  }

  double mutable_value(const double & val) {
    double r = val;
    return r;
  }
 private:
}; // class max_vertex_value
} // namespace pregel
} // namespace paracel

DEFINE_string(server_info, "host1:7777PARACELhost2:8888", "hosts name string of paracel-servers.\n");
DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);

  paracel::json_parser jp(FLAGS_cfg_file);
  std::string input = jp.check_parse<std::string>("input");
  std::string output = jp.parse<std::string>("output");

  auto local_parser = [] (const std::string & line) {
    return paracel::str_split(line, ',');
  };
  auto input_parser = paracel::gen_parser(local_parser);

  auto init_func = [] (const std::string & id) {
    return std::stod(id);
  };
  
  paracel::pregel::max_vertex_value obj(comm, FLAGS_server_info, input, output, input_parser, init_func);
  obj.run_supersteps();
  obj.dump_result("max_vertex_val_");
  return 0;
}
