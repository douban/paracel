##############################--CMakeLists.txt--################################
cmake_minimum_required(VERSION 2.8.9)
################################################################################
project(paracel CXX C)
##############################--g++4.7-plus--###################################
if(EXISTS "gentoo_rules.cmake")
  find_program(CXX NAMES g++-4.9 g++-4.8.1 g++-4.8 g++-4.7.2 g++-4.7.1 g++-4.7 g++)
  execute_process(COMMAND sed -i 
                  "s|CMAKE_CXX_COMPILER [^ ]*|CMAKE_CXX_COMPILER ${CXX}|g" 
                  "gentoo_rules.cmake")
  find_program(CC NAMES gcc-4.9 gcc-4.8.1 gcc-4.8 gcc-4.7.2 gcc-4.7.1 gcc-4.7 gcc)
  execute_process(COMMAND sed -i 
                  "s|CMAKE_C_COMPILER [^ ]*|CMAKE_C_COMPILER ${CC}|g"
                  "gentoo_rules.cmake")
elseif(NOT "${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
  find_program(CMAKE_CXX_COMPILER NAMES g++-4.9 g++-4.8.1 g++-4.8 g++-4.7.2 g++-4.7.1 g++-4.7 g++)
  find_program(CMAKE_C_COMPILER NAMES gcc-4.9 gcc-4.8.1 gcc-4.8 gcc-4.7.2 gcc-4.7.1 gcc-4.7 gcc)
endif()
################################################################################
include(CTest)
##############################--flags--#########################################
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -O2")
  set(CMAKE_CXX_FLAGS_DEBUG "-Wall -std=c++11 -O0 -g -fno-inline")
  set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O3 -DNDEBUG -march=native -ffast-math")
  #set(CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++")
  #set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -stdlib=libc++ -O2")
  set(CMAKE_CXX_FLAGS_DEBUG "-Wall -std=c++11 -stdlib=libc++ -O0 -g -fno-inline")
  set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -stdlib=libc++ -O3 -DNDEBUG -march=native -ffast-math")
endif()

if(CMAKE_BUILD_TYPE MATCHES RELEASE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
endif()

include_directories("${PROJECT_SOURCE_DIR}/include")
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

link_libraries(${Boost_LIBRARIES} ${GFlags_LIBRARIES} ${MsgpackC_LIBRARIES}
               ${ZeroMQ_LIBRARIES} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})
##############################--make-output--###################################
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")
############################--install-path--####################################
install(PROGRAMS prun.py DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
################################################################################
add_subdirectory(src)
add_subdirectory(alg/regression/ridge)
add_subdirectory(alg/classification/logistic_regression)
add_subdirectory(alg/clustering/kmeans)
add_subdirectory(alg/recommendation/matrix_factorization)
add_subdirectory(alg/recommendation/similarity_dense)
add_subdirectory(alg/recommendation/similarity_sparse)
add_subdirectory(alg/recommendation/decision_tree_rec)
add_subdirectory(alg/graph_alg/max_vertex_value)
add_subdirectory(alg/graph_alg/pagerank)
add_subdirectory(alg/topic_model)
add_subdirectory(alg/misc/word_count)
add_subdirectory(test)
add_subdirectory(tool)
################################################################################
