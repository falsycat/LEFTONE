add_library(jukebox
  amp.c
  beep.c
  composite.c
  decoder.c
  delay.c
  effect.c
  format.c
  lowpass.c
  mixer.c
  sound.c
)
target_link_libraries(jukebox
  miniaudio

  math
  memory
)

if (BUILD_TESTING)
  add_executable(jukebox-test test.c)
  target_link_libraries(jukebox-test jukebox)
endif()
