add_executable(sim_dense driver.cpp)
target_link_libraries(sim_dense ${Boost_LIBRARIES} comm scheduler)
install(TARGETS sim_dense RUNTIME DESTINATION bin)
