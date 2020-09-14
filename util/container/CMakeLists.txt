add_library(container
  array.c
)
target_link_libraries(container memory)

if (BUILD_TESTING)
  add_executable(container-test test.c)
  target_link_libraries(container-test container)

  add_test(container-test container-test)
endif()
