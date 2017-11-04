set(ROTATING_FILE_LOG_HANDLE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/RotatingFileLogHandle.cpp)

set(ROTATING_FILE_LOG_HANDLE_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/RotatingFileLogHandle.hpp
        ${CMAKE_SOURCE_DIR}/Project/Logging/AbstractLogHandle.hpp)

add_library(RotatingFileLogHandle SHARED ${ROTATING_FILE_LOG_HANDLE_SOURCES} ${ROTATING_FILE_LOG_HANDLE_HEADERS})
target_include_directories(RotatingFileLogHandle PUBLIC ${CMAKE_SOURCE_DIR}/Project)
target_link_libraries(RotatingFileLogHandle PRIVATE core-config core-utils core-net core-lib Boost::filesystem)