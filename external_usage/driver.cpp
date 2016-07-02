#include <string>
#include <iostream>
#include <unordered_map>

#include <google/gflags.h>

#include "ps.hpp"

namespace paracel {

class demo : public paracel::paralg {

 public:
  demo(paracel::Comm comm,
       std::string hosts_dct_str,
       std::string registery_fn,
       std::string filter_fcn) :
      paracel::paralg(hosts_dct_str, comm),
      registery_file(registery_fn),
      filter_function(filter_fcn) {}
  
  virtual ~demo() {}

  virtual void solve() {
    std::string val = "xxx";
    paracel_write("external_" + std::to_string(get_worker_id()), val);
    paracel_sync();
    if(get_worker_id() == 0) {
      auto D = paracel_read_special<std::string>(registery_file,
                                                 filter_function);
      std::cout << D.size() << std::endl; // expect 1
    }
  }

 private:
  std::string registery_file;
  std::string filter_function;

}; // class demo

} // namespace paracel

DEFINE_string(server_info,
              "host1:7777PARACELhost2:8888",
              "hosts name string of paracel-servers.\n");

DEFINE_string(cfg_file, "", "config json file with absolute path.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);

  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  
  paracel::json_parser pt(FLAGS_cfg_file);
  std::string registery_fn = pt.check_parse<std::string>("registery_file");
  std::string filter_fcn = pt.parse<std::string>("filter_function");

  paracel::demo solver(comm, FLAGS_server_info, registery_fn, filter_fcn);
  solver.solve();
  return 0;
}
