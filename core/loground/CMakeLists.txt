add_library(loground
  base.c
  island.c
  misc.c
  pool.c
)
target_link_libraries(loground
  msgpackc

  math
  memory
  mpkutil

  locommon
  loentity
  loshader
)
