#pragma once

#include <stddef.h>

#include "util/gleasy/program.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_backwall_program_t;

struct loshader_backwall_drawer_t;
typedef struct loshader_backwall_drawer_t loshader_backwall_drawer_t;

typedef enum {
  LOSHADER_BACKWALL_TYPE_WHITE                = 0,
  LOSHADER_BACKWALL_TYPE_INFINITE_BOXES       = 1,
  LOSHADER_BACKWALL_TYPE_HOLLOW_MOUNTAINS     = 2,
  LOSHADER_BACKWALL_TYPE_HOLLOW_MOUNTAINS_RED = 3,
  LOSHADER_BACKWALL_TYPE_JAIL                 = 4,
  LOSHADER_BACKWALL_TYPE_FABRIC               = 5,
} loshader_backwall_type_t;

typedef struct {
  loshader_backwall_type_t type;
  loshader_backwall_type_t prev_type;

  float transition;
} loshader_backwall_drawer_param_t;

void
loshader_backwall_program_initialize(
    loshader_backwall_program_t* prog
);

void
loshader_backwall_program_deinitialize(
    loshader_backwall_program_t* prog
);

loshader_backwall_drawer_t*
loshader_backwall_drawer_new(
    const loshader_backwall_program_t* prog,
    const loshader_uniblock_t*         uniblock
);

void
loshader_backwall_drawer_delete(
    loshader_backwall_drawer_t* drawer
);

void
loshader_backwall_drawer_set_param(
    loshader_backwall_drawer_t*             drawer,
    const loshader_backwall_drawer_param_t* param
);

void
loshader_backwall_drawer_draw(
    const loshader_backwall_drawer_t* drawer
);
