add_executable(als_validate als_validate_driver.cpp)
target_link_libraries(als_validate ${Boost_LIBRARIES} comm scheduler)
install(TARGETS als_validate RUNTIME DESTINATION bin)

add_executable(als als_driver.cpp)
target_link_libraries(als ${Boost_LIBRARIES} comm scheduler)
install(TARGETS als RUNTIME DESTINATION bin)
