set(SOURCES server-test.cpp)

add_test_core(serverTest server-test ${SOURCES})
target_link_libraries(server-test ${Boost_LIBRARIES} {OPENSSL_LIBRARIES})