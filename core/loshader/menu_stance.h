#pragma once

#include <stddef.h>

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_menu_stance_program_t;

struct loshader_menu_stance_drawer_t;
typedef struct loshader_menu_stance_drawer_t loshader_menu_stance_drawer_t;

typedef enum {
  LOSHADER_MENU_STANCE_ID_EMPTY        = 0,
  LOSHADER_MENU_STANCE_ID_MISSIONARY   = 1,
  LOSHADER_MENU_STANCE_ID_REVOLUTIONER = 2,
  LOSHADER_MENU_STANCE_ID_UNFINISHER   = 3,
  LOSHADER_MENU_STANCE_ID_PHILOSOPHER  = 4,
} loshader_menu_stance_id_t;

typedef struct {
  loshader_menu_stance_id_t id;

  vec2_t pos;
  vec2_t size;
  float  alpha;
  float  highlight;
} loshader_menu_stance_drawer_instance_t;

void
loshader_menu_stance_program_initialize(
    loshader_menu_stance_program_t* prog
);

void
loshader_menu_stance_program_deinitialize(
    loshader_menu_stance_program_t* prog
);

loshader_menu_stance_drawer_t*
loshader_menu_stance_drawer_new(
    const loshader_menu_stance_program_t* prog,
    const loshader_uniblock_t*            uniblock
);

void
loshader_menu_stance_drawer_delete(
    loshader_menu_stance_drawer_t* drawer
);

void
loshader_menu_stance_drawer_clear(
    loshader_menu_stance_drawer_t* drawer,
    size_t                         reserve
);

void
loshader_menu_stance_drawer_add_instance(
    loshader_menu_stance_drawer_t*                drawer,
    const loshader_menu_stance_drawer_instance_t* instance
);

void
loshader_menu_stance_drawer_draw(
    const loshader_menu_stance_drawer_t* drawer
);
