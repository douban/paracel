add_library(svd_update SHARED update.cpp)
target_link_libraries(svd_update ${CMAKE_DL_LIBS})
install(TARGETS svd_update LIBRARY DESTINATION lib)

add_executable(svd main.cpp)
target_link_libraries(svd ${Boost_LIBRARIES} comm scheduler)
install(TARGETS svd RUNTIME DESTINATION bin)
