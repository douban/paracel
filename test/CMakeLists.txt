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
