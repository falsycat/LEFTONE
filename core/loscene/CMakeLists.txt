add_library(loscene
  context.c
  game.c
  scene.c
  title.c
)
target_link_libraries(loscene
  msgpackc

  glyphas
  jukebox
  math
  memory
  mpkutil

  locommon
  loentity
  loground
  loparticle
  loplayer
  loresource
  loshader
  loui
  loworld
)
