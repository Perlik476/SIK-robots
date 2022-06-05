cmake_minimum_required(VERSION 3.18)
project(robots)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif ()

set (CMAKE_VERBOSE_MAKEFILE ON)

cmake_host_system_information(RESULT _host_name QUERY HOSTNAME)

if (${_host_name} STREQUAL "students")
    set (LD_LIBRARY_PATH "/opt/gcc-11.2/lib64")
    set (CMAKE_CXX_COMPILER "/opt/gcc-11.2/bin/g++-11.2")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,/opt/gcc-11.2/lib64")

else ()
    set (LD_LIBRARY_PATH "/home/matt/.GCC-11.2/lib64")
    set (CMAKE_CXX_COMPILER "/home/matt/.GCC-11.2/bin/g++")
endif()

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -pthread -std=gnu++20")
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O2")

set (CLIENT_EXECUTABLE ${PROJECT_NAME}-client)
add_executable (${CLIENT_EXECUTABLE}
        src-client/main.cc
        src-client/includes.h
        src-client/data_types/definitions.cc
        src-client/messages.h
        src-client/messages.cc
        src-client/messages/client_messages.h
        src-client/messages/draw_messages.h
        src-client/messages/server_messages.h
        src-client/messages/gui_messages.h
        src-client/data_types/bytes.h
        src-client/data_types/bytes.cc
        src-client/data_types/uint.h
        src-client/data_types/position.h
        src-client/data_types/string.h
        src-client/data_types/list.h
        src-client/data_types/set.h
        src-client/data_types/map.h
        src-client/data_types/player.h
        src-client/data_types/bomb.h
        src-client/data_types/game_state.h
        src-client/data_types/events.h
        src-client/data_types/sockets_info.h
        src-client/data_types/arguments.h
        src-client/data_types/threads_info.h
        src-client/data_types/definitions.h
        src-client/data_types/events.cc)

set (SERVER_EXECUTABLE ${PROJECT_NAME}-server)
add_executable(${SERVER_EXECUTABLE}
        src-server/main.cc
        src-server/includes.h
        src-server/data_types/definitions.cc
        src-server/messages.h
        src-server/messages.cc
        src-server/messages/client_messages.h
        src-server/messages/server_messages.h
        src-server/data_types/bytes.h
        src-server/data_types/bytes.cc
        src-server/data_types/uint.h
        src-server/data_types/position.h
        src-server/data_types/string.h
        src-server/data_types/list.h
        src-server/data_types/set.h
        src-server/data_types/map.h
        src-server/data_types/player.h
        src-server/data_types/bomb.h
        src-server/data_types/game_state.h
        src-server/data_types/events.h
        src-server/data_types/sockets_info.h
        src-server/data_types/arguments.h
        src-server/data_types/threads_info.h
        src-server/data_types/definitions.h
        src-server/data_types/events.cc
        src-server/data_types/arguments.cc
        src-server/data_types/usings.h
        src-server/data_types/game_state.cc
        src-server/messages/client_messages.cc
        src-server/threads.h
        src-server/threads.cpp
        src-server/data_types/action.h
        src-server/data_types/action.cc)

find_package(Boost 1.71 COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

target_link_libraries(${CLIENT_EXECUTABLE} LINK_PUBLIC ${Boost_LIBRARIES})
target_link_libraries(${SERVER_EXECUTABLE} LINK_PUBLIC ${Boost_LIBRARIES})