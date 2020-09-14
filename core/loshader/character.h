#pragma once

#include <stddef.h>
#include <stdint.h>

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_character_program_t;

struct loshader_character_drawer_t;
typedef struct loshader_character_drawer_t loshader_character_drawer_t;

typedef enum {
  LOSHADER_CHARACTER_ID_PLAYER     = 0,
  LOSHADER_CHARACTER_ID_ENCEPHALON = 1,
  LOSHADER_CHARACTER_ID_CAVIA      = 2,
  LOSHADER_CHARACTER_ID_SCIENTIST  = 3,
  LOSHADER_CHARACTER_ID_WARDER     = 4,
} loshader_character_id_t;

typedef enum {
  LOSHADER_CHARACTER_MOTION_ID_STAND1  = 0,
  LOSHADER_CHARACTER_MOTION_ID_STAND2  = 1,
  LOSHADER_CHARACTER_MOTION_ID_WALK    = 2,
  LOSHADER_CHARACTER_MOTION_ID_ATTACK1 = 3,
  LOSHADER_CHARACTER_MOTION_ID_ATTACK2 = 4,
  LOSHADER_CHARACTER_MOTION_ID_SIT     = 5,
  LOSHADER_CHARACTER_MOTION_ID_DOWN    = 6,
} loshader_character_motion_id_t;

typedef struct {
  loshader_character_id_t character_id;

  uint32_t from_motion_id;
  uint32_t to_motion_id;
  float    motion_time;

  float  marker;
  vec2_t marker_offset;

  vec2_t pos;
  vec2_t size;
  vec4_t color;
} loshader_character_drawer_instance_t;

void
loshader_character_program_initialize(
    loshader_character_program_t* prog
);

void
loshader_character_program_deinitialize(
    loshader_character_program_t* prog
);

loshader_character_drawer_t*  /* OWNERSHIP */
loshader_character_drawer_new(
    const loshader_character_program_t* prog,
    const loshader_uniblock_t*          uniblock
);

void
loshader_character_drawer_delete(
    loshader_character_drawer_t* drawer  /* OWNERSHIP */
);

void
loshader_character_drawer_clear(
    loshader_character_drawer_t* drawer,
    size_t                       reserve
);

void
loshader_character_drawer_add_instance(
    loshader_character_drawer_t*                drawer,
    const loshader_character_drawer_instance_t* instance
);

void
loshader_character_drawer_draw(
    const loshader_character_drawer_t* drawer
);
