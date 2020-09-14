add_library(chaos
  abchash.c
  xorshift.c
)
target_link_libraries(chaos
  math
)
