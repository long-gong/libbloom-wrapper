cmake_minimum_required(VERSION 3.0)

project(libbloom-cpp-wrapper)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(./murmur2 ./wyhash)
set(HEADERs bloom.h BloomFilter.h)
add_library(libbloom bloom.c)

add_executable(bf_example example.cpp)
target_link_libraries(bf_example libbloom)