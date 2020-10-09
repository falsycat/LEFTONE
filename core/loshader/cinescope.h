#pragma once

#include <stddef.h>

#include "util/math/vector.h"

#include "./single.h"
#include "./uniblock.h"

typedef struct {
  loshader_single_drawer_t super;
} loshader_cinescope_drawer_t;

typedef struct {
  float  size;
  vec4_t color;
} loshader_cinescope_drawer_param_t;

void
loshader_cinescope_drawer_initialize(
    loshader_cinescope_drawer_t* drawer,
    const loshader_uniblock_t*   uniblock
);

#define loshader_cinescope_drawer_deinitialize(drawer)  \
    loshader_single_drawer_deinitialize(&(drawer)->super)

void
loshader_cinescope_drawer_set_param(
    loshader_cinescope_drawer_t*             drawer,
    const loshader_cinescope_drawer_param_t* param
);

void
loshader_cinescope_drawer_draw(
    const loshader_cinescope_drawer_t* drawer
);
