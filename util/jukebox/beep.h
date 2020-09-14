#pragma once

#include <stdatomic.h>

#include "./effect.h"
#include "./format.h"

typedef struct {
  jukebox_effect_t super;
  jukebox_format_t format;

  atomic_bool playing;

  float hz;
  float amp;
  float phase;
} jukebox_beep_t;

void
jukebox_beep_initialize(
    jukebox_beep_t*         beep,
    const jukebox_format_t* format
);

void
jukebox_beep_deinitialize(
    jukebox_beep_t* beep
);

/* not thread-safe function */
void
jukebox_beep_play(
    jukebox_beep_t* beep,
    float           hz,
    float           amp
);

/* not thread-safe function */
void
jukebox_beep_stop(
    jukebox_beep_t* beep
);
