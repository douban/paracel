add_executable(adjust_ktop_sparse driver.cpp)
target_link_libraries(adjust_ktop_sparse ${Boost_LIBRARIES} comm scheduler)
install(TARGETS adjust_ktop_sparse RUNTIME DESTINATION bin)
