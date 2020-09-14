add_library(locommon
  counter.c
  easing.c
  null.c
  physics.c
  position.c
  ticker.c
)
target_link_libraries(locommon
  msgpackc

  coly2d
  math
  mpkutil
)
