#pragma once

#include <stdatomic.h>

#include "util/math/rational.h"

#include "./effect.h"
#include "./format.h"

typedef struct {
  jukebox_effect_t super;

  jukebox_format_t format;

  atomic_uint_least64_t elapsed;
  atomic_uint_least64_t ease_duration;

  atomic_uint_least16_t prev_amount;  /* [1/1000] */
  atomic_uint_least16_t next_amount;  /* [1/1000] */
} jukebox_amp_t;

void
jukebox_amp_initialize(
    jukebox_amp_t*          amp,
    const jukebox_format_t* format
);

void
jukebox_amp_deinitialize(
    jukebox_amp_t* amp
);

void
jukebox_amp_change_volume(
    jukebox_amp_t*    amp,
    float             amount,
    const rational_t* duration  /* NULLABLE */
);
