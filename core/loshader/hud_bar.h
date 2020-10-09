#pragma once

#include "util/math/vector.h"

#include "./instanced.h"
#include "./uniblock.h"

typedef struct {
  loshader_instanced_drawer_t super;
} loshader_hud_bar_drawer_t;

typedef struct {
  vec2_t pos;
  vec2_t size;

  vec4_t bgcolor;
  vec4_t fgcolor;

  float value;
  float prev_value;
} loshader_hud_bar_drawer_instance_t;

void
loshader_hud_bar_drawer_initialize(
    loshader_hud_bar_drawer_t* drawer,
    const loshader_uniblock_t* uniblock
);

#define loshader_hud_bar_drawer_deinitialize(drawer)  \
    loshader_instanced_drawer_deinitialize(&(drawer)->super)

#define loshader_hud_bar_drawer_clear(drawer, reserve)  \
    loshader_instanced_drawer_clear(&(drawer)->super, reserve)

void
loshader_hud_bar_drawer_add_instance(
    loshader_hud_bar_drawer_t*                drawer,
    const loshader_hud_bar_drawer_instance_t* instance
);

void
loshader_hud_bar_drawer_draw(
    const loshader_hud_bar_drawer_t* drawer
);
