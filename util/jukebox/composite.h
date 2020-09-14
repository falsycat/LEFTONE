#pragma once

#include <stddef.h>

#include "./effect.h"
#include "./format.h"

struct jukebox_composite_t;
typedef struct jukebox_composite_t jukebox_composite_t;

jukebox_composite_t*  /* OWNERSHIP */
jukebox_composite_new(
    const jukebox_format_t* format,
    size_t                  reserve
);

void
jukebox_composite_delete(
    jukebox_composite_t* compo  /* OWNERSHIP */
);

void
jukebox_composite_add_effect(
    jukebox_composite_t* compo,  /* must be stopped */
    jukebox_effect_t*    effect  /* must be alive while the comp is alive */
);

void
jukebox_composite_play(
    jukebox_composite_t* compo
);

void
jukebox_composite_stop(
    jukebox_composite_t* compo
);
