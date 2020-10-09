add_library(flasy
  flasy.c
)
target_link_libraries(flasy
  memory
  tinycthread
)
if (BUILD_TESTING)
  add_executable(flasy-test test.c)
  target_link_libraries(flasy-test flasy)

  set(workdir "${CMAKE_CURRENT_BINARY_DIR}/test")
  file(MAKE_DIRECTORY ${workdir})
  add_test(
      NAME              flasy-test
      COMMAND           bash -c "../flasy-test $(seq 9)"
      WORKING_DIRECTORY ${workdir})
endif()
