#pragma once

#include <stddef.h>

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_bullet_program_t;

struct loshader_bullet_drawer_t;
typedef struct loshader_bullet_drawer_t loshader_bullet_drawer_t;

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
loshader_bullet_program_initialize(
    loshader_bullet_program_t* prog
);

void
loshader_bullet_program_deinitialize(
    loshader_bullet_program_t* prog
);

loshader_bullet_drawer_t*
loshader_bullet_drawer_new(
    const loshader_bullet_program_t* prog,
    const loshader_uniblock_t*       uniblock
);

void
loshader_bullet_drawer_delete(
    loshader_bullet_drawer_t* drawer
);

void
loshader_bullet_drawer_clear(
    loshader_bullet_drawer_t* drawer,
    size_t                    reserve
);

void
loshader_bullet_drawer_add_instance(
    loshader_bullet_drawer_t*                drawer,
    const loshader_bullet_drawer_instance_t* instance
);

void
loshader_bullet_drawer_draw(
    const loshader_bullet_drawer_t* drawer
);
