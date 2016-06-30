##############################--CMakeLists.txt--################################
cmake_minimum_required(VERSION 2.8.9)
################################################################################
project(paracel CXX C)
include(CTest)
##############################--flags--#########################################
include_directories("${PROJECT_SOURCE_DIR}/include")
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAG} -O0 -g -fno-inline")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -stdlib=libc++")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g -fno-inline")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
endif()
##############################--extern-libraries--##############################
find_package(MPI REQUIRED)
include_directories(${MPI_CXX_INCLUDE_PATH})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILER_FLAGS})
set(CMAKE_LINK_FLAGS ${CMAKE_LINK_FLAGS} ${MPI_CXX_LINK_FLAGS})

set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.48.0 REQUIRED COMPONENTS regex filesystem system unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmakes")
find_package(Eigen REQUIRED)
include_directories(${Eigen_INCLUDE_DIR})
find_package(GFlags REQUIRED)
include_directories(${GFlags_INCLUDE_DIR})
find_package(MsgpackC REQUIRED)
include_directories(${MsgpackC_INCLUDE_DIR})
find_package(ZeroMQ REQUIRED)
include_directories(${ZermMQ_INCLUDE_DIR})
find_package(Glog REQUIRED)
include_directories(${Glog_INCLUDE_DIR})

link_libraries(${Boost_LIBRARIES} ${GFlags_LIBRARIES} ${MsgpackC_LIBRARIES}
               ${ZeroMQ_LIBRARIES} ${Glog_LIBRARIES} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})
##############################--make-output--###################################
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")
############################--install-path--####################################
install(PROGRAMS prun.py DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
install(DIRECTORY include DESTINATION "${CMAKE_INSTALL_PREFIX}")
################################################################################
add_subdirectory(src)
add_subdirectory(alg/regression/ridge)
add_subdirectory(alg/classification/logistic_regression)
add_subdirectory(alg/clustering/kmeans)
add_subdirectory(alg/recommendation/matrix_factorization)
add_subdirectory(alg/recommendation/similarity_dense)
add_subdirectory(alg/recommendation/similarity_sparse)
add_subdirectory(alg/recommendation/decision_tree_rec)
add_subdirectory(alg/recommendation/als)
add_subdirectory(alg/recommendation/adjust_ktop_sparse)
add_subdirectory(alg/graph_alg/max_vertex_value)
add_subdirectory(alg/graph_alg/pagerank)
add_subdirectory(alg/topic_model)
add_subdirectory(alg/misc/word_count)
add_subdirectory(test)
add_subdirectory(tool)
################################################################################
