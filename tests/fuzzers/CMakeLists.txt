if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR APPLE)
  message(STATUS "Skipping fuzzers on non-clang or apple")
  return()
endif()

set(FUZZTEST_FUZZING_MODE "ON" CACHE STRING "Fuzzing mode" FORCE)

FetchContent_Declare(
  fuzztest
  GIT_REPOSITORY https://github.com/google/fuzztest.git
  GIT_TAG        2024-10-28
)

FetchContent_MakeAvailable(fuzztest)

enable_testing()
fuzztest_setup_fuzzing_flags()

file(GLOB_RECURSE FUZZER_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(
  driftsort-fuzzer
  ${FUZZER_SOURCES}
)

target_link_libraries(driftsort-fuzzer PRIVATE gtest driftsort)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT APPLE)
message(STATUS "enable address sanitizer for fuzzers on non-apple clang")
  target_compile_options(driftsort-fuzzer PRIVATE -fsanitize=address -DADDRESS_SANITIZER)
  target_link_options(driftsort-fuzzer PRIVATE -fsanitize=address)
endif()


link_fuzztest(driftsort-fuzzer)
gtest_discover_tests(driftsort-fuzzer)
