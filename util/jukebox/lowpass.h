#pragma once

#include <stdatomic.h>

#include "./effect.h"
#include "./format.h"

#define JUKEBOX_LOWPASS_SUPPORT_MAX_CHANNELS 16

#define JUKEBOX_LOWPASS_FACTOR_UNIT (1.0f/10000)

typedef struct {
  jukebox_effect_t super;
  jukebox_format_t format;

  atomic_uint_least32_t factor;

  float prev[JUKEBOX_LOWPASS_SUPPORT_MAX_CHANNELS];
} jukebox_lowpass_t;

void
jukebox_lowpass_initialize(
    jukebox_lowpass_t*      lowpass,
    const jukebox_format_t* format,
    float                   factor
);

void
jukebox_lowpass_deinitialize(
    jukebox_lowpass_t* lowpass
);
