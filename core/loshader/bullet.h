#pragma once

#include <stddef.h>

#include "util/math/vector.h"

#include "./instanced.h"
#include "./uniblock.h"

typedef struct {
  loshader_instanced_drawer_t super;
} loshader_bullet_drawer_t;

typedef enum {
  LOSHADER_BULLET_ID_LIGHT    = 0,
  LOSHADER_BULLET_ID_SQUARE   = 1,
  LOSHADER_BULLET_ID_TRIANGLE = 2,
} loshader_bullet_id_t;

typedef struct {
  loshader_bullet_id_t bullet_id;

  vec2_t pos;
  vec2_t size;
  float  theta;
  float  time;

  vec4_t color;
} loshader_bullet_drawer_instance_t;

void
loshader_bullet_drawer_initialize(
    loshader_bullet_drawer_t*  drawer,
    const loshader_uniblock_t* uniblock
);

#define loshader_bullet_drawer_deinitialize(drawer)  \
    loshader_instanced_drawer_deinitialize(&(drawer)->super)

#define loshader_bullet_drawer_clear(drawer, reserve)  \
    loshader_instanced_drawer_clear(&(drawer)->super, reserve)

void
loshader_bullet_drawer_add_instance(
    loshader_bullet_drawer_t*                drawer,
    const loshader_bullet_drawer_instance_t* instance
);

void
loshader_bullet_drawer_draw(
    const loshader_bullet_drawer_t* drawer
);
