#pragma once

#include <stddef.h>

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_combat_ring_program_t;

struct loshader_combat_ring_drawer_t;
typedef struct loshader_combat_ring_drawer_t loshader_combat_ring_drawer_t;

typedef struct {
  float  range;
  float  start;
  float  end;
  vec4_t color;
} loshader_combat_ring_drawer_instance_t;

void
loshader_combat_ring_program_initialize(
    loshader_combat_ring_program_t* prog
);

void
loshader_combat_ring_program_deinitialize(
    loshader_combat_ring_program_t* prog
);

loshader_combat_ring_drawer_t*  /* OWNERSHIP */
loshader_combat_ring_drawer_new(
    const loshader_combat_ring_program_t* prog,
    const loshader_uniblock_t*            uniblock
);

void
loshader_combat_ring_drawer_delete(
    loshader_combat_ring_drawer_t* drawer  /* OWNERSHIP */
);

void
loshader_combat_ring_drawer_clear(
    loshader_combat_ring_drawer_t* drawer,
    size_t                         reserve
);

void
loshader_combat_ring_drawer_add_instance(
    loshader_combat_ring_drawer_t*                drawer,
    const loshader_combat_ring_drawer_instance_t* instance
);

void
loshader_combat_ring_drawer_draw(
    const loshader_combat_ring_drawer_t* drawer
);
