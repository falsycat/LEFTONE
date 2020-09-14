add_library(parsarg
  parsarg.c
)

if (BUILD_TESTING)
  add_executable(parsarg-test test.c)
  target_link_libraries(parsarg-test
    parsarg

    conv
  )
endif()
