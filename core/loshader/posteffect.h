#pragma once

#include "util/gleasy/framebuffer.h"

#include "./single.h"
#include "./uniblock.h"

typedef struct {
  loshader_single_drawer_t    super;
  const gleasy_framebuffer_t* fb;
} loshader_posteffect_drawer_t;

typedef struct {
  /* distortion effect */
  float distortion_amnesia;
  float distortion_radial;
  float distortion_urgent;
  float raster_whole;

  /* color effect */
  float aberration_radial;
  float blur_whole;
  float brightness_whole;
  float fade_radial;
} loshader_posteffect_drawer_param_t;

void
loshader_posteffect_drawer_initialize(
    loshader_posteffect_drawer_t* drawer,
    const loshader_uniblock_t*    uniblock,
    const gleasy_framebuffer_t*   fb
);

#define loshader_posteffect_drawer_deinitialize(drawer)  \
    loshader_single_drawer_deinitialize(&(drawer)->super)

void
loshader_posteffect_drawer_set_param(
    loshader_posteffect_drawer_t*             drawer,
    const loshader_posteffect_drawer_param_t* param
);

void
loshader_posteffect_drawer_draw(
    const loshader_posteffect_drawer_t* drawer
);
