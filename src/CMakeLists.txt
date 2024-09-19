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

# nothing works without preCICE
find_package( precice REQUIRED CONFIG )
find_package( yaml-cpp REQUIRED )

## Older yaml-cpp builds do not configure the default import target, so go for it:
if( NOT TARGET yaml-cpp::yaml-cpp )
  if( TARGET yaml-cpp )
    add_library( yaml-cpp::yaml-cpp ALIAS yaml-cpp )
  else()
    message( FATAL_ERROR "Could not configure yaml-cpp" )
  endif()
endif()


# Add the executable
set( APPNAME ts_dummy_adaptor )
add_executable( ${APPNAME}
  main.cpp
  ForceGenerator.cpp
  yaml/Settings.cpp
  yaml/parse.cpp )

target_link_libraries(
  ${APPNAME}
  PRIVATE precice::precice yaml-cpp::yaml-cpp )
