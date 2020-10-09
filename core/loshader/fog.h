#pragma once

#include "util/math/vector.h"

#include "core/locommon/position.h"

#include "./single.h"
#include "./uniblock.h"

typedef struct {
  loshader_single_drawer_t super;
} loshader_fog_drawer_t;

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
loshader_fog_drawer_initialize(
    loshader_fog_drawer_t*     drawer,
    const loshader_uniblock_t* uniblock
);

#define loshader_fog_drawer_deinitialize(drawer)  \
    loshader_single_drawer_deinitialize(&(drawer)->super)

void
loshader_fog_drawer_set_param(
    loshader_fog_drawer_t*             drawer,
    const loshader_fog_drawer_param_t* param
);

void
loshader_fog_drawer_draw(
    const loshader_fog_drawer_t* drawer
);
