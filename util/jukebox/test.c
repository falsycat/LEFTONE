#undef NDEBUG

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/math/rational.h"

#include "./delay.h"
#include "./effect.h"
#include "./lowpass.h"
#include "./mixer.h"
#include "./sound.h"

int main(int argc, char** argv) {
  assert(argc == 2);

  static const jukebox_format_t format = {
    .sample_rate = 48000,
    .channels    = 2,
  };

  jukebox_sound_buffer_t* sndbuf =
      jukebox_sound_buffer_new_from_file(&format, argv[1]);

  jukebox_sound_t* snd = jukebox_sound_new(sndbuf, 16);

  static const rational_t delay_dur = rational(1, 10);
  jukebox_delay_t* delay = jukebox_delay_new(&format, &delay_dur, 0.1f, 0.5f);

  jukebox_lowpass_t lowpass = {0};
  jukebox_lowpass_initialize(&lowpass, &format, 0.01f);

  jukebox_mixer_t* mixer = jukebox_mixer_new(&format, 16);
  jukebox_mixer_add_effect(mixer, (jukebox_effect_t*) snd);
  jukebox_mixer_add_effect(mixer, (jukebox_effect_t*) delay);
  jukebox_mixer_add_effect(mixer, &lowpass.super);

  for (size_t i = 0; i < 32; ++i) {
    jukebox_sound_play(snd);
    printf("press enter to continue...\n");
    getchar();
  }
  jukebox_sound_stop_all(snd);

  jukebox_mixer_delete(mixer);
  jukebox_lowpass_deinitialize(&lowpass);
  jukebox_delay_delete(delay);
  jukebox_sound_delete(snd);
  jukebox_sound_buffer_delete(sndbuf);

  return EXIT_SUCCESS;
}
