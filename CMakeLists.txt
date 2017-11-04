cmake_minimum_required(VERSION 3.7)
project(spider_server)

##START CONFIG CMAKE
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/vendor/core/cmake-utils/Module)
set(CMAKE_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/vendor/core/cmake-utils)

include(${CMAKE_CONFIG_PATH}/include.cmake)

add_subdirectory(vendor/core)

PROJECT_FOLDER(${PROJECT_NAME})
STANDARD_COMPILATION_SETTINGS()
CPPREST_CONFIG()
BOOST_CONFIG_SPIDER_SERVER()
OPENSSL_CONFIG()
find_package(Threads)
if (VERBOSE)
    PRINT_BUILD_TYPE()
    PRINT_FLAGS_INFO()
endif ()
enable_testing()
include(CTest)

add_subdirectory(${CMAKE_SOURCE_DIR}/Project/Logging/Modules/RotatingFileLogHandle)
add_subdirectory(${CMAKE_SOURCE_DIR}/Project/Logging/Modules/APIConnectedLogHandle)

set(SOURCES_PROTOCOL
        Project/Protocol/MessagesEnums.hpp
        Project/Protocol/Messages.hpp
        Project/Protocol/CommandHandler.hpp
        )

set(SOURCES_LOGGING
        Project/Logging/LogManager.hpp
        Project/Logging/AbstractLogHandle.hpp
        )

set(SOURCES_NETWORK
        Project/Network/BufferView.hpp
        Project/Network/ErrorCode.hpp
        Project/Network/IOManager.hpp
        Project/Network/TCPAcceptor.hpp
        Project/Network/SSLContext.hpp
        Project/Network/SSLConnection.hpp
        )

set(SOURCES_SERVER
        Project/Server/CommandableSession.hpp
        Project/Server/SpiderClientSession.hpp
        Project/Server/ShellClientSession.hpp
        Project/Server/Server.hpp
        )

set(SOURCES_UTILS
        Project/Utils/Serialization.hpp
        Project/Utils/Stringification.hpp
        Project/Utils/JSONification.hpp
        Project/Utils/ILoggable.hpp
        )

set(SOURCES_SPIDER
        ${SOURCES_PROTOCOL}
        ${SOURCES_LOGGING}
        ${SOURCES_NETWORK}
        ${SOURCES_SERVER}
        ${SOURCES_UTILS}
        Project/Core/SpiderCore.hpp
        Project/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES_SPIDER})

add_dependencies(${PROJECT_NAME} RotatingFileLogHandle)
add_dependencies(${PROJECT_NAME} APIConnectedLogHandle)
include_directories(Project)
#include_directories(vendor/core/Project/core-utils/project/)
#include_directories(vendor/core/Project/core-network/project/)
target_link_libraries(${PROJECT_NAME} PUBLIC core-log core-net core-lib PRIVATE ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} PUBLIC Project)
#add_subdirectory(test)