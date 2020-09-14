add_library(loworld
  chunk.c
  environment.c
  generator.c
  poolset.c
  store.c
  template.c
  view.c
)
target_link_libraries(loworld
  msgpackc

  chaos
  container
  jukebox
  math
  memory
  mpkutil

  lobullet
  locharacter
  locommon
  loentity
  loground
  loresource
  loshader
)
