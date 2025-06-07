cmake_minimum_required(VERSION 3.16)

project(ArgParser 
    VERSION 1.0.0
    DESCRIPTION "A modern C++ argument parser library"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(argparser
    src/ArgParser.cpp
    src/Argument.cpp
)

target_include_directories(argparser
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Create an alias for consistent naming
add_library(argparser::argparser ALIAS argparser)

message(STATUS "")
message(STATUS "ArgParser Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")