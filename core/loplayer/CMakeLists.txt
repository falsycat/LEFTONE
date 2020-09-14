add_library(loplayer
  action.c
  camera.c
  combat.c
  controller.c
  entity.c
  event.c
  hud.c
  menu.c
  player.c
  status.c
)
target_link_libraries(loplayer
  msgpackc

  conv
  glyphas
  math
  memory
  mpkutil

  lobullet
  locommon
  loeffect
  loentity
  loresource
  loshader
)
