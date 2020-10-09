#pragma once

#include <stddef.h>
#include <stdint.h>

#include "util/math/vector.h"

#include "./instanced.h"
#include "./uniblock.h"

typedef struct {
  loshader_instanced_drawer_t super;
} loshader_particle_drawer_t;

typedef enum {
  LOSHADER_PARTICLE_ID_PLAYER     = 0,
  LOSHADER_PARTICLE_ID_ENCEPHALON = 1,
  LOSHADER_PARTICLE_ID_CAVIA      = 2,
  LOSHADER_PARTICLE_ID_SCIENTIST  = 3,
  LOSHADER_PARTICLE_ID_WARDER     = 4,
} loshader_particle_id_t;

typedef struct {
  loshader_particle_id_t particle_id;
  vec2_t pos;
  vec2_t size;
  vec4_t color;
  float  time;
} loshader_particle_drawer_instance_t;

void
loshader_particle_drawer_initialize(
    loshader_particle_drawer_t* drawer,
    const loshader_uniblock_t*  uniblock
);

#define loshader_particle_drawer_deinitialize(drawer)  \
    loshader_instanced_drawer_deinitialize(&(drawer)->super)

#define loshader_particle_drawer_clear(drawer, reserve)  \
    loshader_instanced_drawer_clear(&(drawer)->super, reserve)

void
loshader_particle_drawer_add_instance(
    loshader_particle_drawer_t*                drawer,
    const loshader_particle_drawer_instance_t* instance
);

void
loshader_particle_drawer_draw(
    const loshader_particle_drawer_t* drawer
);
