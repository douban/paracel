add_library(gLDA_handle SHARED handle.cpp)
target_link_libraries(gLDA_handle ${CMAKE_DL_LIBS})
install(TARGETS gLDA_handle LIBRARY DESTINATION lib)

add_executable(gLDA gLDA_driver.cpp)
target_link_libraries(gLDA ${Boost_LIBRARIES} comm scheduler)
install(TARGETS gLDA RUNTIME DESTINATION bin)
