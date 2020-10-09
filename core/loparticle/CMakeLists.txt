add_library(loparticle
  aura.c
  base.c
  misc.c
  pool.c
)
target_crial_sources(loparticle
  aura.crial
  base.crial
)
target_link_libraries(loparticle
  msgpackc

  mpkutil

  locommon
  loentity
  loshader
)
