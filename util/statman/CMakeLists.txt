add_library(statman
  statman.c
)

if (BUILD_TESTING)
  add_executable(statman-test test.c)
  target_link_libraries(statman-test statman)
  add_test(statman-test statman-test)
endif()
