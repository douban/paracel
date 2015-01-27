# Find gflags installed local, and then system
# - Try to find gflags
# Once done, this will be defined:
#  GFlags_INCLUDE_DIR - include to use gflags in paracel
#  GFlags_LIBRARIES - link to use gflags in paracel

find_path(GFlags_INCLUDE_DIR google/gflags.h
  NO_DEFAULT_PATH
  PATHS
  "/usr/local/include"
  "/usr/include")

find_library(GFlags_LIBRARY
  NAMES gflags libgflags
  HINTS 
  "/user/local/lib"
  "/usr/lib")

message(STATUS "Find GFlags include path: ${GFlags_INCLUDE_DIR}")
message(STATUS "Find GFlags lib path: ${GFlags_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GFlags
  REQUIRED_VARS
  GFlags_LIBRARY
  GFlags_INCLUDE_DIR)

set(GFlags_LIBRARIES ${GFlags_LIBRARY})

mark_as_advanced(
  GFlags_LIBRARIES
  GFlags_INCLUDE_DIR)
