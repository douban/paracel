add_library(wc_update SHARED handle.cpp)
target_link_libraries(wc_update ${CMAKE_DL_LIBS})
install(TARGETS wc_update LIBRARY DESTINATION lib)

add_executable(wc wc_driver.cpp)
target_link_libraries(wc ${Boost_LIBRARIES} ${Glog_LIBRARIES} comm scheduler)
install(TARGETS wc RUNTIME DESTINATION bin)
