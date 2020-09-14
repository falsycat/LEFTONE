#pragma once

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_hud_bar_program_t;

struct loshader_hud_bar_drawer_t;
typedef struct loshader_hud_bar_drawer_t loshader_hud_bar_drawer_t;

typedef struct {
  vec2_t pos;
  vec2_t size;

  vec4_t bgcolor;
  vec4_t fgcolor;

  float value;
  float prev_value;
} loshader_hud_bar_drawer_instance_t;

void
loshader_hud_bar_program_initialize(
    loshader_hud_bar_program_t* prog
);

void
loshader_hud_bar_program_deinitialize(
    loshader_hud_bar_program_t* prog
);

loshader_hud_bar_drawer_t*  /* OWNERSHIP */
loshader_hud_bar_drawer_new(
    const loshader_hud_bar_program_t* prog,
    const loshader_uniblock_t*        uniblock
);

void
loshader_hud_bar_drawer_delete(
    loshader_hud_bar_drawer_t* drawer  /* OWNERSHIP */
);

void
loshader_hud_bar_drawer_clear(
    loshader_hud_bar_drawer_t* drawer,
    size_t                     reserve
);

void
loshader_hud_bar_drawer_add_instance(
    loshader_hud_bar_drawer_t*                drawer,
    const loshader_hud_bar_drawer_instance_t* instance
);

void
loshader_hud_bar_drawer_draw(
    const loshader_hud_bar_drawer_t* drawer
);
