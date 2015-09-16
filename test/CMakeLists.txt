include(CTest)
enable_testing()

add_executable(test_compile test_compile.cpp)
target_link_libraries(test_compile ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_compile COMMAND test_compile)
install(TARGETS test_compile RUNTIME DESTINATION bin/test)

add_executable(test_utils test_utils.cpp)
target_link_libraries(test_utils ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_utils COMMAND test_utils)
install(TARGETS test_utils RUNTIME DESTINATION bin/test)
install(FILES "${PROJECT_SOURCE_DIR}/test/test.json" DESTINATION bin/test)

add_executable(test_partition test_partition.cpp)
target_link_libraries(test_partition ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_partition COMMAND test_partition)
install(TARGETS test_partition RUNTIME DESTINATION bin/test)

#add_executable(test_server test_server.cpp)
#target_link_libraries(test_server ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
#add_test(NAME test_server COMMAND test_server)
#install(TARGETS test_server RUNTIME DESTINATION bin/test)

add_executable(test_graph test_graph.cpp)
target_link_libraries(test_graph ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_graph COMMAND test_graph)
install(TARGETS test_graph RUNTIME DESTINATION bin/test)

add_executable(test_comm test_comm.cpp)
target_link_libraries(test_comm comm ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
install(TARGETS test_comm RUNTIME DESTINATION bin/test)

#add_executable(test_client test_client.cpp)
#target_link_libraries(test_client ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
#add_test(NAME test_client COMMAND test_client)
#install(TARGETS test_client RUNTIME DESTINATION bin/test)

#add_executable(test_init_client test_init_client.cpp)
#target_link_libraries(test_init_client ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
#add_test(NAME test_init_client COMMAND test_init_client)
#install(TARGETS test_init_client RUNTIME DESTINATION bin/test)

add_executable(test_proxy test_proxy.cpp)
target_link_libraries(test_proxy ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_proxy COMMAND test_proxy)
install(TARGETS test_proxy RUNTIME DESTINATION bin/test)

add_executable(test_kv test_kv.cpp)
target_link_libraries(test_kv ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_kv COMMAND test_kv)
install(TARGETS test_kv RUNTIME DESTINATION bin/test)

add_executable(test_ring test_ring.cpp)
target_link_libraries(test_ring ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_ring COMMAND test_ring)
install(TARGETS test_ring RUNTIME DESTINATION bin/test)

add_executable(test_f_traits test_f_traits.cpp)
target_link_libraries(test_f_traits ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_f_traits COMMAND test_f_traits)
install(TARGETS test_f_traits RUNTIME DESTINATION bin/test)

add_executable(test_decomp test_decomp.cpp)
target_link_libraries(test_decomp ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_decomp COMMAND test_decomp)
install(TARGETS test_decomp RUNTIME DESTINATION bin/test)

add_executable(test_paracel_types test_paracel_types.cpp)
target_link_libraries(test_paracel_types ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_paracel_types COMMAND test_paracel_types)
install(TARGETS test_paracel_types RUNTIME DESTINATION bin/test)

add_executable(test_tree_reduce test_tree_reduce.cpp)
target_link_libraries(test_tree_reduce comm ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
install(TARGETS test_tree_reduce RUNTIME DESTINATION bin/test)

add_executable(test_paste test_paste.cpp)
target_link_libraries(test_paste ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_paste COMMAND test_paste)
install(TARGETS test_paste RUNTIME DESTINATION bin/test)

add_executable(test_packer test_packer.cpp)
target_link_libraries(test_packer ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_packer COMMAND test_packer)
install(TARGETS test_packer RUNTIME DESTINATION bin/test)

add_executable(test_glog test_glog.cpp)
target_link_libraries(test_glog comm ${CMAKE_DL_LIBS})
add_test(NAME test_glog COMMAND test_glog)
install(TARGETS test_glog RUNTIME DESTINATION bin/test)
