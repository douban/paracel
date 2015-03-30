add_library(lr_update SHARED update.cpp)
target_link_libraries(lr_update ${CMAKE_DL_LIBS})
install(TARGETS lr_update LIBRARY DESTINATION lib)

add_library(lr_method SHARED lr.cpp)
target_link_libraries(lr_method ${Boost_LIBRARIES} comm scheduler)
install(TARGETS lr_method LIBRARY DESTINATION lib)

add_executable(lr lr_driver.cpp)
target_link_libraries(lr
  ${Boost_LIBRARIES} 
  comm scheduler lr_method)
install(TARGETS lr RUNTIME DESTINATION bin)
