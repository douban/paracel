# Find eigen installed local, and then system
# - Try to find eigen(version >=3.0)
# Once done, this will be defined:
#  Eigen_INCLUDE_DIR - include to use eigen in paracel

include(CheckCXXSourceRuns)

find_path(Eigen_INCLUDE_DIR eigen3/Eigen/Dense
  NO_DEFAULT_PATH
  PATHS
  "/usr/local/include"
  "/usr/include")

message(STATUS "Find Eigen include path: ${Eigen_INCLUDE_DIR}")

set(CMAKE_REQUIRED_INCLUDES ${Eigen_INCLUDE_DIR})
set(CMAKE_REQUIRED_FLAGS)
check_cxx_source_runs("
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
int main(int argc, char *argv[])
{
  Eigen::VectorXd tmp(7);
  return 0;
}
" Eigen_CHECK_FINE)
message(STATUS "Eigen check: ${Eigen_CHECK_FINE}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Eigen
  REQUIRED_VARS
  Eigen_INCLUDE_DIR
  Eigen_CHECK_FINE)

mark_as_advanced(Eigen_INCLUDE_DIR)
