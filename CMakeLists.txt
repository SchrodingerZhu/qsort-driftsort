cmake_minimum_required(VERSION 3.12)
project(driftsort-cpp LANGUAGES CXX)

include(CMakeDependentOption)
set(CMAKE_CXX_STANDARD 17)

option(DRIFTSORT_BUILD_TESTS "Build tests" ON)
cmake_dependent_option(DRIFTSORT_BUILD_FUZZERS "Build fuzzers" ON "DRIFTSORT_BUILD_TESTS" OFF)

if (DRIFTSORT_BUILD_TESTS)
  add_subdirectory(tests)
endif()
