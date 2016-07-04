# Find msgpack-C installed local, and then system
# - Try to find Msgpack-C
# Once done, this will be defined:
#  MsgpackC_INCLUDE_DIR - include to use Msgpack-C in paracel

include(CheckCXXSourceRuns)

find_path(MsgpackC_INCLUDE_DIR msgpack.hpp
  NO_DEFAULT_PATH
  PATHS
  "/usr/local/include"
  "/usr/include")

message(STATUS "Find Mespack-C include path: ${MsgpackC_INCLUDE_DIR}")

set(CMAKE_REQUIRED_INCLUDES ${MsgpackC_INCLUDE_DIR})
set(CMAKE_REQUIRED_FLAGS)
check_cxx_source_runs("
#include <vector>
#include <msgpack.hpp>
int main(int argc, char *argv[])
{
  std::vector<std::string> target;
  target.push_back(\"Hello,\");
  target.push_back(\"World!\");
  msgpack::sbuffer sbuf;  // simple buffer
  msgpack::pack(&sbuf, target);
  msgpack::unpacked msg;    // includes memory pool and deserialized object
  msgpack::unpack(&msg, sbuf.data(), sbuf.size());
  msgpack::object obj = msg.get();
  std::vector<std::string> result;
  obj.convert(&result);
  return 0;
}
" MsgpackC_CHECK_FINE)
message(STATUS "MsgpackC check: ${MsgpackC_CHECK_FINE}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  MsgpackC
  REQUIRED_VARS
  MsgpackC_INCLUDE_DIR
  MsgpackC_CHECK_FINE)

mark_as_advanced(
  MsgpackC_INCLUDE_DIR)
