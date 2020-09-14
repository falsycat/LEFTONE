add_library(lobullet
  base.c
  bomb.c
  linear.c
  misc.c
  pool.c
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
