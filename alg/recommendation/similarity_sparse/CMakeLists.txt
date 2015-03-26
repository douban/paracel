add_executable(sim_sparse sim_sparse_driver.cpp)
target_link_libraries(sim_sparse ${BoostLIBRARIES} comm scheduler)
install(TARGETS sim_sparse RUNTIME DESTINATION bin)
