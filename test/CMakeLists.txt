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

add_executable(test_server test_server.cpp)
target_link_libraries(test_server ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
add_test(NAME test_server COMMAND test_server)
install(TARGETS test_server RUNTIME DESTINATION bin/test)

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

