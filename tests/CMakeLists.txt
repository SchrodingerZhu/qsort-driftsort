include(FetchContent)

add_library(driftsort_test INTERFACE)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)

FetchContent_MakeAvailable(googletest)

if(DRIFTSORT_BUILD_FUZZERS)
  if (MSVC)
    message(WARNING "Fuzzers are not supported on MSVC")
  elseif (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(WARNING "Fuzzers are only supported on Clang")
  else()
    add_subdirectory(fuzzers)
  endif()
endif()
