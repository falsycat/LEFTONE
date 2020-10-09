add_library(loplayer
  camera.c
  combat.c
  controller.c
  event.c
  player.c
  popup.c
  stance.c
)
target_benum_sources(loplayer
  stance.h
)
target_crial_sources(loplayer
  event.crial
  player.crial
)
target_link_libraries(loplayer
  msgpackc

  math
  mpkutil

  locommon
  loentity
  loshader
)
