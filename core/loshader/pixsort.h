#pragma once

#include "util/gleasy/framebuffer.h"
#include "util/gleasy/program.h"

#include "./single.h"
#include "./uniblock.h"

typedef struct {
  loshader_single_drawer_t    super;
  const gleasy_framebuffer_t* fb;

  /* public mutable params */
  float intensity;
} loshader_pixsort_drawer_t;

void
loshader_pixsort_drawer_initialize(
    loshader_pixsort_drawer_t*  drawer,
    const loshader_uniblock_t*  uniblock,
    const gleasy_framebuffer_t* fb
);

#define loshader_pixsort_drawer_deinitialize(drawer)  \
    loshader_single_drawer_deinitialize(&(drawer)->super)

void
loshader_pixsort_drawer_draw(
    const loshader_pixsort_drawer_t* drawer
);

bool
loshader_pixsort_drawer_is_skippable(
    const loshader_pixsort_drawer_t* drawer
);
