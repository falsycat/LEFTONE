add_library(lobullet
  base.c
  linear.c
  pool.c
)
target_benum_sources(lobullet
  type.h
)
target_crial_sources(lobullet
  base.crial
)
target_link_libraries(lobullet
  msgpackc

  math
  memory
  mpkutil
  coly2d

  locommon
  loeffect
  loentity
  loshader
)
