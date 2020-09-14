add_library(locharacter
  base.c
  big_warder.c
  cavia.c
  encephalon.c
  greedy_scientist.c
  misc.c
  pool.c
  scientist.c
  theists_child.c
  util.c
  warder.c
)
target_link_libraries(locharacter
  msgpackc

  math
  memory
  mpkutil

  lobullet
  locommon
  loeffect
  loentity
  loplayer
  loshader
)
