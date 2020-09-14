add_library(mpkutil
  file.c
  get.c
  pack.c
)
target_link_libraries(mpkutil
  msgpackc

  math
)
