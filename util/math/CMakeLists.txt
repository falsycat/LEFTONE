add_library(math
  rational.c
  vector.c
)
target_source_of_source(math
  algorithm.sos.c
  matrix.sos.c
)
target_link_libraries(math m)

if (BUILD_TESTING)
  add_executable(math-test test.c)
  target_link_libraries(math-test math)

  add_test(math-test math-test)
endif()
