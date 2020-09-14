#pragma once

#include <stdint.h>

#include "util/jukebox/format.h"
#include "util/jukebox/mixer.h"
#include "util/math/rational.h"

struct loresource_sound_t;
typedef struct loresource_sound_t loresource_sound_t;

loresource_sound_t*  /* OWNERSHIP */
loresource_sound_new(
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format
);

void
loresource_sound_delete(
    loresource_sound_t* sound  /* OWNERSHIP */
);

void
loresource_sound_play(
    loresource_sound_t* sound,
    const char*         name
);

void
loresource_sound_change_master_volume(
    loresource_sound_t* sound,
    float               v,
    const rational_t*   duration
);
