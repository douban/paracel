add_executable(lr_serial lr_serial.cpp)
target_link_libraries(lr_serial ${Boost_LIBRARIES} comm scheduler)
install(TARGETS lr_serial RUNTIME DESTINATION bin/tool)

add_executable(lasso_serial lasso_serial.cpp)
target_link_libraries(lasso_serial ${Boost_LIBRARIES} comm scheduler)
install(TARGETS lasso_serial RUNTIME DESTINATION bin/tool)

add_executable(lasso_serial_rigid lasso_serial_rigid.cpp)
target_link_libraries(lasso_serial_rigid ${Boost_LIBRARIES} comm scheduler)
install(TARGETS lasso_serial_rigid RUNTIME DESTINATION bin/tool)

add_executable(lr_l1_serial lr_l1_serial.cpp)
target_link_libraries(lr_l1_serial ${Boost_LIBRARIES} comm scheduler)
install(TARGETS lr_l1_serial RUNTIME DESTINATION bin/tool)

add_executable(kmeans_serial kmeans_serial.cpp)
target_link_libraries(kmeans_serial ${Boost_LIBRARIES} comm scheduler)
install(TARGETS kmeans_serial RUNTIME DESTINATION bin/tool)

add_executable(svd_serial svd_serial.cpp)
target_link_libraries(svd_serial ${Boost_LIBRARIES} comm scheduler)
install(TARGETS svd_serial RUNTIME DESTINATION bin/tool)

add_executable(gLDA_serial gLDA_serial.cpp)
target_link_libraries(gLDA_serial ${Boost_LIBRARIES} comm scheduler)
install(TARGETS gLDA_serial RUNTIME DESTINATION bin/tool)

install(FILES datagen.py DESTINATION bin/tool)
