#pragma once

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./instanced.h"
#include "./uniblock.h"

typedef struct {
  loshader_instanced_drawer_t super;
} loshader_ground_drawer_t;

typedef enum {
  LOSHADER_GROUND_ID_ISLAND = 0,
} loshader_ground_id_t;

typedef struct {
  loshader_ground_id_t ground_id;

  vec2_t pos;
  vec2_t size;
} loshader_ground_drawer_instance_t;

void
loshader_ground_drawer_initialize(
    loshader_ground_drawer_t*  drawer,
    const loshader_uniblock_t* uniblock
);

#define loshader_ground_drawer_deinitialize(drawer)  \
    loshader_instanced_drawer_deinitialize(&(drawer)->super)

#define loshader_ground_drawer_clear(drawer, reserve)  \
    loshader_instanced_drawer_clear(&(drawer)->super, reserve)

void
loshader_ground_drawer_add_instance(
    loshader_ground_drawer_t*                drawer,
    const loshader_ground_drawer_instance_t* instance
);

void
loshader_ground_drawer_draw(
    const loshader_ground_drawer_t* drawer
);
