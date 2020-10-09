add_library(lochara
  base.c
  big_warder.c
  cavia.c
  combat.c
  encephalon.c
  player.c
  pool.c
  state.c
  strategy.c
  theists_child.c
  warder.c
)
target_benum_sources(lochara
  state.h
  strategy.h
  type.h
)
target_crial_sources(lochara
  base.crial
)

target_link_libraries(lochara
  msgpackc

  chaos
  math
  statman

  lobullet
  locommon
  loeffect
  loentity
  loplayer
  loresource
  loshader
)
