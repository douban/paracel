add_executable(als als_driver.cpp)
target_link_libraries(als ${Boost_LIBRARIES} comm scheduler)
install(TARGETS als RUNTIME DESTINATION bin)
