set(ROTATING_FILE_LOG_HANDLE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/RotatingFileLogModule.cpp)

set(ROTATING_FILE_LOG_HANDLE_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/RotatingFileLogModule.hpp
        ${CMAKE_SOURCE_DIR}/Project/Logging/LogModule.hpp)

add_library(RotatingFileLogModule SHARED ${ROTATING_FILE_LOG_HANDLE_SOURCES} ${ROTATING_FILE_LOG_HANDLE_HEADERS})
target_include_directories(RotatingFileLogModule PUBLIC ${CMAKE_SOURCE_DIR}/Project)
target_link_libraries(RotatingFileLogModule PRIVATE core-config core-utils core-net core-lib Boost::filesystem)