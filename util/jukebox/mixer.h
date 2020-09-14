#pragma once

#include <stddef.h>

#include "./effect.h"
#include "./format.h"

struct jukebox_mixer_t;
typedef struct jukebox_mixer_t jukebox_mixer_t;

jukebox_mixer_t*  /* OWNERSHIP */
jukebox_mixer_new(
    const jukebox_format_t* format,
    size_t                  reserve
);

void
jukebox_mixer_delete(
    jukebox_mixer_t* mixer  /* OWNERSHIP */
);

void
jukebox_mixer_add_effect(
    jukebox_mixer_t*  mixer,
    jukebox_effect_t* effect  /* must be alive while the mixer is alive */
);
