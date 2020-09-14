#pragma once

#include <stddef.h>

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_ground_program_t;

struct loshader_ground_drawer_t;
typedef struct loshader_ground_drawer_t loshader_ground_drawer_t;

typedef enum {
  LOSHADER_GROUND_ID_ISLAND = 0,
} loshader_ground_id_t;

typedef struct {
  loshader_ground_id_t ground_id;

  vec2_t pos;
  vec2_t size;
} loshader_ground_drawer_instance_t;

void
loshader_ground_program_initialize(
    loshader_ground_program_t* prog
);

void
loshader_ground_program_deinitialize(
    loshader_ground_program_t* prog
);

loshader_ground_drawer_t*
loshader_ground_drawer_new(
    const loshader_ground_program_t* prog,
    const loshader_uniblock_t*       uniblock
);

void
loshader_ground_drawer_delete(
    loshader_ground_drawer_t* drawer
);

void
loshader_ground_drawer_clear(
    loshader_ground_drawer_t* drawer,
    size_t                    reserve
);

void
loshader_ground_drawer_add_instance(
    loshader_ground_drawer_t*                drawer,
    const loshader_ground_drawer_instance_t* instance
);

void
loshader_ground_drawer_draw(
    const loshader_ground_drawer_t* drawer
);
