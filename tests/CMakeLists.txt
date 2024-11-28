include(FetchContent)

add_library(driftsort_test INTERFACE)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)

FetchContent_MakeAvailable(googletest)

if(DRIFTSORT_BUILD_FUZZERS)
  add_subdirectory(fuzzers)
endif()
