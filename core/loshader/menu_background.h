#pragma once

#include "util/gleasy/program.h"

#include "./single.h"
#include "./uniblock.h"

typedef struct {
  loshader_single_drawer_t super;

  /* public mutable params */
  float alpha;
} loshader_menu_background_drawer_t;

void
loshader_menu_background_drawer_initialize(
    loshader_menu_background_drawer_t* drawer,
    const loshader_uniblock_t*         uniblock
);

#define loshader_menu_background_drawer_deinitialize(drawer)  \
    loshader_single_drawer_deinitialize(&(drawer)->super)

void
loshader_menu_background_drawer_draw(
    const loshader_menu_background_drawer_t* drawer
);
