#pragma once

#include <stddef.h>
#include <stdint.h>

#include "util/math/vector.h"

#include "./instanced.h"
#include "./uniblock.h"

typedef struct {
  loshader_instanced_drawer_t super;
} loshader_character_drawer_t;

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

  struct {
    loshader_character_motion_id_t from;
    loshader_character_motion_id_t to;
    float                          time;
  } motion;

  float  marker;
  vec2_t marker_offset;

  vec2_t pos;
  vec2_t size;
  vec4_t color;
} loshader_character_drawer_instance_t;

void
loshader_character_drawer_initialize(
    loshader_character_drawer_t* drawer,
    const loshader_uniblock_t*   uniblock
);

#define loshader_character_drawer_deinitialize(drawer)  \
    loshader_instanced_drawer_deinitialize(&(drawer)->super)

#define loshader_character_drawer_clear(drawer, reserve)  \
    loshader_instanced_drawer_clear(&(drawer)->super, reserve)

void
loshader_character_drawer_add_instance(
    loshader_character_drawer_t*                drawer,
    const loshader_character_drawer_instance_t* instance
);

void
loshader_character_drawer_draw(
    const loshader_character_drawer_t* drawer
);
