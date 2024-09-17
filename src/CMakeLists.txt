# Minimum required CMake version
cmake_minimum_required(VERSION 3.15)

# Set the project name and version
project(TS_FORCE_GENERATOR VERSION 1.0 LANGUAGES CXX)

# Check if the build is happening inside the source directory
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not allowed. Please use a separate build directory.")
endif()

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the executable
add_executable( ts_adaptor
  main.cpp
  ForceGenerator.cpp )
