add_executable(sim_dense driver.cpp)
target_link_libraries(sim_dense ${Boost_LIBRARIES} comm scheduler)
install(TARGETS sim_dense RUNTIME DESTINATION bin)

add_executable(ab_sim_dense ab_driver.cpp)
target_link_libraries(ab_sim_dense ${Boost_LIBRARIES} comm scheduler)
install(TARGETS ab_sim_dense RUNTIME DESTINATION bin)
