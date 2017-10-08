cmake_minimum_required(VERSION 3.7)
project(spider_server)

##START CONFIG CMAKE
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-utils/Module)
set(CMAKE_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-utils)

include(${CMAKE_CONFIG_PATH}/include.cmake)

PROJECT_FOLDER(${PROJECT_NAME})
STANDARD_COMPILATION_SETTINGS()
BOOST_CONFIG_SPIDER_SERVER()
OPENSSL_CONFIG()
find_package(Threads)
if (VERBOSE)
    PRINT_BUILD_TYPE()
    PRINT_FLAGS_INFO()
endif ()
enable_testing()
include(CTest)

set(SOURCES_PROTOCOL
        Project/Protocol/MessagesEnums.hpp
        Project/Protocol/Messages.hpp
        Project/Protocol/CommandHandler.hpp
        )

set(SOURCES_LOGGING
        Project/Logging/LogManager.hpp
        Project/Logging/AbstractLogHandle.hpp
        Project/Logging/RotatingFileLogHandle.hpp
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
include_directories(Project)
include_directories(vendor/core/Project/core-utils/project/)
include_directories(vendor/core/Project/core-network/project/)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_subdirectory(test)