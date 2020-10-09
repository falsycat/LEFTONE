add_library(loeffect
  effect.c
  generic.c
  recipient.c
)
target_crial_sources(loeffect
  recipient.crial
)
target_link_libraries(loeffect
  msgpackc

  mpkutil

  locommon
  loshader
)
