set(API_CONNECTED_LOG_HANDLE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/APIConnectedLogModule.cpp)

set(API_CONNECTED_LOG_HANDLE_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/APIConnectedLogModule.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/API.hpp
        ${CMAKE_SOURCE_DIR}/Project/Logging/LogModule.hpp)

add_library(APIConnectedLogModule SHARED ${API_CONNECTED_LOG_HANDLE_SOURCES} ${API_CONNECTED_LOG_HANDLE_HEADERS})
target_include_directories(APIConnectedLogModule PUBLIC ${CMAKE_SOURCE_DIR}/Project)
target_link_libraries(APIConnectedLogModule PRIVATE core-config core-utils core-lib core-net cpprestsdk::cpprest)