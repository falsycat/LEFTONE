#pragma once

#include <stddef.h>

#include "util/gleasy/framebuffer.h"
#include "util/gleasy/program.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_posteffect_program_t;

struct loshader_posteffect_drawer_t;
typedef struct loshader_posteffect_drawer_t loshader_posteffect_drawer_t;

typedef struct {
  float whole_blur;
  float raster;

  float radial_displacement;
  float amnesia_displacement;
  float radial_fade;

  float brightness;
} loshader_posteffect_drawer_param_t;

void
loshader_posteffect_program_initialize(
    loshader_posteffect_program_t* prog
);

void
loshader_posteffect_program_deinitialize(
    loshader_posteffect_program_t* prog
);

loshader_posteffect_drawer_t*  /* OWNERSHIP */
loshader_posteffect_drawer_new(
    const loshader_posteffect_program_t* prog,
    const loshader_uniblock_t*           uniblock,
    const gleasy_framebuffer_t*          fb
);

void
loshader_posteffect_drawer_delete(
    loshader_posteffect_drawer_t* drawer  /* OWNERSHIP */
);

void
loshader_posteffect_drawer_set_param(
    loshader_posteffect_drawer_t*             drawer,
    const loshader_posteffect_drawer_param_t* param
);

void
loshader_posteffect_drawer_draw(
    const loshader_posteffect_drawer_t* drawer
);
