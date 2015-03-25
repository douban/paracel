add_library(ridge_update SHARED update.cpp)
target_link_libraries(ridge_update ${CMAKE_DL_LIBS})
install(TARGETS ridge_update LIBRARY DESTINATION lib)

add_executable(ridge_regression ridge_driver.cpp)
target_link_libraries(ridge_regression ${Boost_LIBRARIES} comm scheduler)
install(TARGETS ridge_regression RUNTIME DESTINATION bin)
