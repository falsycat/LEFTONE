#pragma once

#include <stddef.h>

#include "./single.h"
#include "./uniblock.h"

typedef struct {
  loshader_single_drawer_t super;
} loshader_backwall_drawer_t;

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
loshader_backwall_drawer_initialize(
    loshader_backwall_drawer_t* drawer,
    const loshader_uniblock_t*  uniblock
);

#define loshader_backwall_drawer_deinitialize(drawer)  \
    loshader_single_drawer_deinitialize(&(drawer)->super)

void
loshader_backwall_drawer_set_param(
    loshader_backwall_drawer_t*             drawer,
    const loshader_backwall_drawer_param_t* param
);

void
loshader_backwall_drawer_draw(
    const loshader_backwall_drawer_t* drawer
);
