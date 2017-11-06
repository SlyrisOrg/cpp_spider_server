set(SOURCES_REMOTE Project/main.cpp)
set(HEADERS_REMOTE Project/Client.hpp Project/ClientSession.hpp Project/PosixStream.hpp Project/SyncCommandableSession.hpp)
add_executable(remote-shell ${SOURCES_REMOTE} ${HEADERS_REMOTE})
target_link_libraries(remote-shell PUBLIC core-utils core-log core-net PRIVATE ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})