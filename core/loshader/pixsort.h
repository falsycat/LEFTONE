#pragma once

#include <stddef.h>

#include "util/gleasy/framebuffer.h"
#include "util/gleasy/program.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_pixsort_program_t;

struct loshader_pixsort_drawer_t;
typedef struct loshader_pixsort_drawer_t loshader_pixsort_drawer_t;

void
loshader_pixsort_program_initialize(
    loshader_pixsort_program_t* prog
);

void
loshader_pixsort_program_deinitialize(
    loshader_pixsort_program_t* prog
);

loshader_pixsort_drawer_t*  /* OWNERSHIP */
loshader_pixsort_drawer_new(
    const loshader_pixsort_program_t* prog,
    const loshader_uniblock_t*        uniblock,
    const gleasy_framebuffer_t*       fb
);

void
loshader_pixsort_drawer_delete(
    loshader_pixsort_drawer_t* drawer  /* OWNERSHIP */
);

void
loshader_pixsort_drawer_set_intensity(
    loshader_pixsort_drawer_t* drawer,
    float                      intensity
);

void
loshader_pixsort_drawer_draw(
    const loshader_pixsort_drawer_t* drawer
);

bool
loshader_pixsort_drawer_is_skippable(
    const loshader_pixsort_drawer_t* drawer
);
