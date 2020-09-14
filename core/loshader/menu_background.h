#pragma once

#include "util/gleasy/program.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_menu_background_program_t;

struct loshader_menu_background_drawer_t;
typedef
    struct loshader_menu_background_drawer_t
    loshader_menu_background_drawer_t;

void
loshader_menu_background_program_initialize(
    loshader_menu_background_program_t* prog
);

void
loshader_menu_background_program_deinitialize(
    loshader_menu_background_program_t* prog
);

loshader_menu_background_drawer_t*
loshader_menu_background_drawer_new(
    const loshader_menu_background_program_t* prog,
    const loshader_uniblock_t*                uniblock
);

void
loshader_menu_background_drawer_delete(
    loshader_menu_background_drawer_t* drawer
);

void
loshader_menu_background_drawer_set_alpha(
    loshader_menu_background_drawer_t* drawer,
    float                              alpha
);

void
loshader_menu_background_drawer_draw(
    const loshader_menu_background_drawer_t* drawer
);
