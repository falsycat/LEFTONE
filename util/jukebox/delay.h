#pragma once

#include <stddef.h>

#include "util/math/rational.h"

#include "./format.h"

struct jukebox_delay_t;
typedef struct jukebox_delay_t jukebox_delay_t;

jukebox_delay_t*
jukebox_delay_new(
    const jukebox_format_t* format,
    const rational_t*       duration,
    float                   source_attenuation,
    float                   feedback_attenuation
);

void
jukebox_delay_delete(
    jukebox_delay_t* delay
);
