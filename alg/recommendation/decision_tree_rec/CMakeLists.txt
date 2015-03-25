add_library(dtr_update SHARED handle.cpp)
target_link_libraries(dtr_update ${CMAKE_DL_LIBS})
install(TARGETS dtr_update LIBRARY DESTINATION lib)

add_executable(dtr driver.cpp)
target_link_libraries(dtr ${Boost_LIBRARIES} comm scheduler)
install(TARGETS dtr RUNTIME DESTINATION bin)
