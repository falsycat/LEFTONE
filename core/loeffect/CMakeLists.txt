add_library(loeffect
  effect.c
  generic.c
  recipient.c
  stance.c
)
target_link_libraries(loeffect
  msgpackc

  mpkutil

  locommon
  loshader
)
