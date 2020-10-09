add_library(loground
  base.c
  island.c
  pool.c
)
target_benum_sources(loground
  type.h
)
target_crial_sources(loground
  base.crial
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
