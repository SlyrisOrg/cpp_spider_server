set(SOURCES serializer-test.cpp)

add_test_core(serializerTest serializer-test ${SOURCES})
target_link_libraries(serializer-test core-utils core-net)