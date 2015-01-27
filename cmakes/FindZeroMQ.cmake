# Find zeromq installed local, and then system
# - Try to find ZeroMQ(version >=3.2)
# Once done, this will be defined:
#  ZeroMQ_INCLUDE_DIR - include to use ZeroMQ in paracel
#  ZeroMQ_LIBRARIES - link to use ZeroMQ in paracel

include(CheckCXXSourceRuns)

find_path(ZeroMQ_INCLUDE_DIR zmq.hpp
  NO_DEFAULT_PATH
  PATHS
  "/usr/local/include"
  "/usr/include")

find_library(ZeroMQ_LIBRARY
  NAMES zmq libzmq
  HINTS
  "/user/local/lib"
  "/usr/lib")

message(STATUS "Find ZeroMQ include path: ${ZeroMQ_INCLUDE_DIR}")
message(STATUS "Find ZeroMQ lib path: ${ZeroMQ_LIBRARY}")

set(CMAKE_REQUIRED_INCLUDES ${ZeroMQ_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${ZeroMQ_LIBRARY})
set(CMAKE_REQUIRED_FLAGS -std=c++11)
check_cxx_source_runs("
#include <zmq.hpp>
int main(int argc, char *argv[])
{
  zmq::context_t context(1);
  zmq::socket_t sock(context, ZMQ_REP);
  char port[1024]; size_t size = sizeof(port);
  sock.bind(\"tcp://*:*\");
  sock.getsockopt(ZMQ_LAST_ENDPOINT, &port, &size);
  return 0;
}
" ZeroMQ_CHECK_FINE)
message(STATUS "ZeroMQ advanced module check: ${ZeroMQ_CHECK_FINE}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  ZeroMQ
  REQUIRED_VARS
  ZeroMQ_LIBRARY
  ZeroMQ_INCLUDE_DIR
  ZeroMQ_CHECK_FINE)

set(ZeroMQ_LIBRARIES ${ZeroMQ_LIBRARY})

mark_as_advanced(
  ZeroMQ_LIBRARIES
  ZeroMQ_INCLUDE_DIR)
