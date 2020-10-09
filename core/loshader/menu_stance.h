#pragma once

#include "util/math/vector.h"

#include "./instanced.h"
#include "./uniblock.h"

typedef struct {
  loshader_instanced_drawer_t super;
} loshader_menu_stance_drawer_t;

typedef enum {
  LOSHADER_MENU_STANCE_ID_EMPTY        = 0,
  LOSHADER_MENU_STANCE_ID_MISSIONARY   = 1,
  LOSHADER_MENU_STANCE_ID_REVOLUTIONER = 2,
  LOSHADER_MENU_STANCE_ID_UNFINISHER   = 3,
  LOSHADER_MENU_STANCE_ID_PHILOSOPHER  = 4,
  LOSHADER_MENU_STANCE_ID_BETRAYER     = 0,  /* NOT IMPELEMENTED */
} loshader_menu_stance_id_t;

typedef struct {
  loshader_menu_stance_id_t id;

  vec2_t pos;
  vec2_t size;
  float  alpha;
} loshader_menu_stance_drawer_instance_t;

void
loshader_menu_stance_drawer_initialize(
    loshader_menu_stance_drawer_t* drawer,
    const loshader_uniblock_t*     uniblock
);

#define loshader_menu_stance_drawer_deinitialize(drawer)  \
    loshader_instanced_drawer_deinitialize(&(drawer)->super)

#define loshader_menu_stance_drawer_clear(drawer, reserve)  \
    loshader_instanced_drawer_clear(&(drawer)->super, reserve)

void
loshader_menu_stance_drawer_add_instance(
    loshader_menu_stance_drawer_t*                drawer,
    const loshader_menu_stance_drawer_instance_t* instance
);

void
loshader_menu_stance_drawer_draw(
    const loshader_menu_stance_drawer_t* drawer
);
