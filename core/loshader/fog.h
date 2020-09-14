#pragma once

#include "util/gleasy/program.h"

#include "core/locommon/position.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_fog_program_t;

typedef struct loshader_fog_drawer_t loshader_fog_drawer_t;

typedef enum {
  LOSHADER_FOG_TYPE_NONE        = 0,
  LOSHADER_FOG_TYPE_WHITE_CLOUD = 1,
} loshader_fog_type_t;

typedef struct {
  loshader_fog_type_t type;
  loshader_fog_type_t prev_type;

  float transition;

  float               bounds_fog;
  locommon_position_t bounds_pos;
  vec2_t              bounds_size;
} loshader_fog_drawer_param_t;

void
loshader_fog_program_initialize(
    loshader_fog_program_t* prog
);

void
loshader_fog_program_deinitialize(
    loshader_fog_program_t* prog
);

loshader_fog_drawer_t*
loshader_fog_drawer_new(
    const loshader_fog_program_t* prog,
    const loshader_uniblock_t*    uniblock
);

void
loshader_fog_drawer_delete(
    loshader_fog_drawer_t* drawer
);

void
loshader_fog_drawer_set_param(
    loshader_fog_drawer_t*             drawer,
    const loshader_fog_drawer_param_t* param
);

void
loshader_fog_drawer_draw(
    const loshader_fog_drawer_t* drawer
);
