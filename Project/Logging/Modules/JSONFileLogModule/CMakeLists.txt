set(JSON_FILE_LOG_HANDLE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/JSONFileLogModule.cpp)

set(JSON_FILE_LOG_HANDLE_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/JSONFileLogModule.hpp
        ${CMAKE_SOURCE_DIR}/Project/Logging/LogModule.hpp)

add_library(JSONFileLogModule SHARED ${JSON_FILE_LOG_HANDLE_SOURCES} ${JSON_FILE_LOG_HANDLE_HEADERS})
target_include_directories(JSONFileLogModule PUBLIC ${CMAKE_SOURCE_DIR}/Project)
target_link_libraries(JSONFileLogModule PRIVATE core-config core-utils core-net core-lib Boost::filesystem)