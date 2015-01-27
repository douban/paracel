add_library(mf_update SHARED update.cpp)
target_link_libraries(mf_update ${CMAKE_DL_LIBS})
install(TARGETS mf_update LIBRARY DESTINATION lib)

add_library(mf_filter SHARED filter.cpp)
target_link_libraries(mf_filter ${CMAKE_DL_LIBS})
install(TARGETS mf_filter LIBRARY DESTINATION lib)

add_executable(mf mf_driver.cpp)
target_link_libraries(mf ${Boost_LIBRARIES} comm scheduler)
install(TARGETS mf RUNTIME DESTINATION bin)
