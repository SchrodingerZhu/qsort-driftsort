file(GLOB_RECURSE UNITTESTS_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(
  driftsort-unitests
  ${UNITTESTS_SOURCES}
)

target_link_libraries(driftsort-unitests PRIVATE GTest::gtest_main driftsort)

include(GoogleTest)
gtest_discover_tests(driftsort-unitests)
