cmake_minimum_required(VERSION 3.12)
project(driftsort-cpp LANGUAGES CXX)

include(CMakeDependentOption)
set(CMAKE_CXX_STANDARD 20)

option(DRIFTSORT_BUILD_TESTS "Build tests" ON)
option(DRIFTSORT_BUILD_BENCHMARKS "Build benchmarks" ON)
cmake_dependent_option(DRIFTSORT_BUILD_FUZZERS "Build fuzzers" ON "DRIFTSORT_BUILD_TESTS" OFF)

add_library(driftsort INTERFACE)
target_include_directories(driftsort INTERFACE include)

add_subdirectory(src)

if (DRIFTSORT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if (DRIFTSORT_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()


