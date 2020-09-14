add_library(memory
  memory.c
)

if (BUILD_TESTING)
  add_executable(memory-test test.c)
  target_link_libraries(memory-test memory)

  add_test(memory-test memory-test)
endif()
