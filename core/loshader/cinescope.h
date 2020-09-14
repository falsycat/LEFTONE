#pragma once

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_cinescope_program_t;

struct loshader_cinescope_drawer_t;
typedef struct loshader_cinescope_drawer_t loshader_cinescope_drawer_t;

typedef struct {
  float  size;
  vec4_t color;
} loshader_cinescope_drawer_param_t;

void
loshader_cinescope_program_initialize(
    loshader_cinescope_program_t* prog
);

void
loshader_cinescope_program_deinitialize(
    loshader_cinescope_program_t* prog
);

loshader_cinescope_drawer_t*
loshader_cinescope_drawer_new(
    const loshader_cinescope_program_t* prog,
    const loshader_uniblock_t*          uniblock
);

void
loshader_cinescope_drawer_delete(
    loshader_cinescope_drawer_t* drawer
);

void
loshader_cinescope_drawer_set_param(
    loshader_cinescope_drawer_t*             drawer,
    const loshader_cinescope_drawer_param_t* param
);

void
loshader_cinescope_drawer_draw(
    const loshader_cinescope_drawer_t* drawer
);
