cmake_minimum_required(VERSION 2.8)

project(p4t_native)

find_package(OpenMP)

set(CMAKE_CXX_FLAGS 
    "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -std=c++17 -Wall -Werror -O3\
    -msse4.1")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(PythonLibs 3.6 REQUIRED)

find_package(Boost COMPONENTS python3 numpy3 REQUIRED)
include_directories( ${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRES} ${CMAKE_SOURCE_DIR} )

add_subdirectory(p4t)
