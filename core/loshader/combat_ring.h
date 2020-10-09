#pragma once

#include "util/math/vector.h"

#include "./instanced.h"
#include "./uniblock.h"

typedef struct {
  loshader_instanced_drawer_t super;
} loshader_combat_ring_drawer_t;

typedef struct {
  float  range;
  float  start;
  float  end;
  vec4_t color;
} loshader_combat_ring_drawer_instance_t;

void
loshader_combat_ring_drawer_initialize(
    loshader_combat_ring_drawer_t* drawer,
    const loshader_uniblock_t*     uniblock
);

#define loshader_combat_ring_drawer_deinitialize(drawer)  \
    loshader_instanced_drawer_deinitialize(&(drawer)->super)

#define loshader_combat_ring_drawer_clear(drawer, reserve)  \
    loshader_instanced_drawer_clear(&(drawer)->super, reserve)

void
loshader_combat_ring_drawer_add_instance(
    loshader_combat_ring_drawer_t*                drawer,
    const loshader_combat_ring_drawer_instance_t* instance
);

void
loshader_combat_ring_drawer_draw(
    const loshader_combat_ring_drawer_t* drawer
);
