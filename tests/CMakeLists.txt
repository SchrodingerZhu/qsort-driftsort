include(FetchContent)

add_library(driftsort_test INTERFACE)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)

FetchContent_MakeAvailable(googletest)

if(DRIFTSORT_BUILD_FUZZERS)
  if (MSVC OR APPLE)
    message(STATUS "Skipping fuzzers on MSVC or Apple")
  elseif (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Skipping fuzzers on non-clang compiler")
  else()
    add_subdirectory(fuzzers)
  endif()
endif()
