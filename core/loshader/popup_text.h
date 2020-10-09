#pragma once

#include "util/gleasy/texture.h"

#include "./text.h"
#include "./uniblock.h"

typedef gleasy_program_t loshader_popup_text_program_t;

typedef struct {
  loshader_text_drawer_t super;

  /* public mutable params */
  float alpha;
} loshader_popup_text_drawer_t;

void
loshader_popup_text_drawer_initialize(
    loshader_popup_text_drawer_t* drawer,
    const loshader_uniblock_t*    uniblock,
    gleasy_texture_2d_t           tex
);

#define loshader_popup_text_drawer_deinitialize(drawer)  \
    loshader_text_drawer_deinitialize(&(drawer)->super)

#define loshader_popup_text_drawer_clear(drawer, reserve)  \
    loshader_text_drawer_clear(&(drawer)->super, reserve)

#define loshader_popup_text_drawer_add_block(drawer, block)  \
    loshader_text_drawer_add_block(&(drawer)->super, block)

void
loshader_popup_text_drawer_draw(
    const loshader_popup_text_drawer_t* drawer
);
