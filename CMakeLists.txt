cmake_minimum_required(VERSION 3.18)
project(robots)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

# We want to see make commands.
set (CMAKE_VERBOSE_MAKEFILE ON)

cmake_host_system_information(RESULT _host_name QUERY HOSTNAME)
message("Hostname is: " ${_host_name})

# Settings GCC 11.2 compiler
if (${_host_name} STREQUAL "students")
    message ("Setting STUDENTS gcc11.2 path")
    message ("Before running your program run: export LD_LIBRARY_PATH=/opt/gcc-11.2/lib64")
    set (LD_LIBRARY_PATH "/opt/gcc-11.2/lib64")
    set (CMAKE_CXX_COMPILER "/opt/gcc-11.2/bin/g++-11.2")
else ()
    message ("Setting LOCAL gcc11.2 path")
    message ("Before running your program run: export LD_LIBRARY_PATH=/home/matt/.GCC-11.2/lib64")
    set (LD_LIBRARY_PATH "/home/matt/.GCC-11.2/lib64")
    set (CMAKE_CXX_COMPILER "/home/matt/.GCC-11.2/bin/g++")
endif()

# Set flags
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -pthread -std=gnu++20") #-Werror
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O2")

# Add executable client
set (CLIENT_EXECUTABLE ${PROJECT_NAME}-client)
add_executable (${CLIENT_EXECUTABLE}
        src-client/main.cpp
        )

# Add executable server
set (SERVER_EXECUTABLE ${PROJECT_NAME}-server)
add_executable (${SERVER_EXECUTABLE}
        src-server/main.cpp
        )

# Setup BOOST library
find_package(Boost 1.71 COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost Not found")
endif()

target_link_libraries(${CLIENT_EXECUTABLE} LINK_PUBLIC ${Boost_LIBRARIES})
target_link_libraries(${SERVER_EXECUTABLE} LINK_PUBLIC ${Boost_LIBRARIES})