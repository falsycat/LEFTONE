add_library(dictres
  dictres.c
)
target_link_libraries(dictres
  chaos
)

if (BUILD_TESTING)
  add_executable(dictres-test test.c)
  target_link_libraries(dictres-test dictres)
endif()
