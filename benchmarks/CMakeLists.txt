include(FetchContent)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.1
)

FetchContent_MakeAvailable(benchmark)

file(GLOB_RECURSE BENCHMARKS_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(
  driftsort-benchmarks
  ${BENCHMARKS_SOURCES}
)

target_link_libraries(driftsort-benchmarks PRIVATE benchmark::benchmark driftsort)
