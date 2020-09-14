add_library(loentity
  bullet.c
  character.c
  entity.c
  store.c
)
target_link_libraries(loentity
  msgpackc

  coly2d
  container
  math
  memory

  locommon
)
