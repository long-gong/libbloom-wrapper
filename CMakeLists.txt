cmake_minimum_required(VERSION 3.0)

project(libbloom-cpp-wrapper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

include_directories(./murmur2 ./wyhash)
set(HEADERs bloom.h BloomFilter.h)
add_library(libbloom bloom.c ./murmur2/MurmurHash2.c)

add_executable(bf_example example.cpp bloom.c ./murmur2/MurmurHash2.c)

add_executable(bf_perf benchmark/benchmarks.cpp bloom.c ./murmur2/MurmurHash2.c)
target_include_directories(bf_perf PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(bf_perf bf)

IF (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/libbloom")
    message("Downloading libbloom")
    execute_process(COMMAND git clone https://github.com/jvirkki/libbloom.git
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    if (result)
        message(FATAL_ERROR "CMake step for downloading libbloom failed: ${result}")
    endif ()
endif ()

add_executable(bf_libbloom_org_perf benchmark/benchmark_libbloom_org.cpp ${CMAKE_CURRENT_BINARY_DIR}/libbloom/bloom.c ${CMAKE_CURRENT_BINARY_DIR}/libbloom/murmur2/MurmurHash2.c)
target_include_directories(bf_libbloom_org_perf PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()
add_executable(bf_test tests/BloomFilterTest.cpp bloom.c murmur2/MurmurHash2.c)
target_link_libraries(bf_test gtest_main gtest pthread)
target_compile_definitions(bf_test PUBLIC -DDEBUG)
target_include_directories(bf_test PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
add_test(NAME bf_test COMMAND bf_test)
add_test(NAME bf_valgrind_test
        COMMAND sh -c "valgrind --leak-check=yes --error-exitcode=1 $<TARGET_FILE:bf_test>")


