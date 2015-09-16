find_path(Glog_INCLUDE_DIR glog/logging.h
  NO_DEFAULT_PATH
  PATHS
  "/usr/local/include"
  "/usr/include"  
)

find_library(Glog_LIBRARY
  NAMES glog libglog
  HINTS
  "/usr/local/lib"
  "/usr/lib")

message(STATUS "Find Glog include path: ${Glog_INCLUDE_DIR}")
message(STATUS "Find Glog lib path: ${Glog_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Glog
  REQUIRED_VARS
  Glog_LIBRARY
  Glog_INCLUDE_DIR)

set(Glog_LIBRARIES ${Glog_LIBRARY})
    
mark_as_advanced(
  Glog_LIBRARIES
  Glog_INCLUDE_DIR)
