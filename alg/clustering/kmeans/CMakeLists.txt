add_library(kmeans_update SHARED update.cpp)
target_link_libraries(kmeans_update ${CMAKE_DL_LIBS})
install(TARGETS kmeans_update LIBRARY DESTINATION lib)

add_executable(kmeans kmeans_driver.cpp)
target_link_libraries(kmeans ${Boost_LIBRARIES} comm scheduler)
install(TARGETS kmeans RUNTIME DESTINATION bin)
