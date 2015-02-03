add_library(pagerank_handle SHARED handle.cpp)
target_link_libraries(pagerank_handle ${CMAKE_DL_LIBS})
install(TARGETS pagerank_handle LIBRARY DESTINATION lib)

add_executable(pagerank main.cpp)
target_link_libraries(pagerank ${Boost_LIBRARIES} comm scheduler)
install(TARGETS pagerank RUNTIME DESTINATION bin)
