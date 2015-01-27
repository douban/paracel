add_library(mvv_update SHARED update.cpp)
target_link_libraries(mvv_update ${CMAKE_DL_LIBS})
install(TARGETS mvv_update LIBRARY DESTINATION lib)

add_executable(mvv max_vertex_value_driver.cpp)
target_link_libraries(mvv ${Boost_LIBRARIES} comm scheduler)
install(TARGETS mvv RUNTIME DESTINATION bin)

#add_executable(mvv_pregel max_vertex_value_pregel.cpp)
#target_link_libraries(mvv_pregel ${Boost_LIBRARIES} comm scheduler)
#install(TARGETS mvv_pregel RUNTIME DESTINATION bin)
