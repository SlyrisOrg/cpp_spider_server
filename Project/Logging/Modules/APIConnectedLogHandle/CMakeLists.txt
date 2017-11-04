set(API_CONNECTED_LOG_HANDLE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/APIConnectedLogHandle.cpp)

set(API_CONNECTED_LOG_HANDLE_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/APIConnectedLogHandle.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/API.hpp
        ${CMAKE_SOURCE_DIR}/Project/Logging/AbstractLogHandle.hpp)

add_library(APIConnectedLogHandle SHARED ${API_CONNECTED_LOG_HANDLE_SOURCES} ${API_CONNECTED_LOG_HANDLE_HEADERS})
target_include_directories(APIConnectedLogHandle PUBLIC ${CMAKE_SOURCE_DIR}/Project)
target_link_libraries(APIConnectedLogHandle PRIVATE core-config core-utils core-lib core-net cpprestsdk::cpprest)