include(FetchContent)

if (MSVC)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)

FetchContent_MakeAvailable(googletest)

if(DRIFTSORT_BUILD_FUZZERS)
  if (MSVC)
    message(STATUS "Skipping fuzzers on MSVC")
  else()
    add_subdirectory(fuzzers)
  endif()
endif()

add_subdirectory(unittests)
