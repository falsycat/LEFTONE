#pragma once

#include "util/gleasy/texture.h"

#include "./text.h"
#include "./uniblock.h"

typedef struct {
  loshader_text_drawer_t super;
} loshader_event_line_drawer_t;

void
loshader_event_line_drawer_initialize(
    loshader_event_line_drawer_t* drawer,
    const loshader_uniblock_t*    uniblock,
    gleasy_texture_2d_t           tex
);

#define loshader_event_line_drawer_deinitialize(drawer)  \
    loshader_text_drawer_deinitialize(&(drawer)->super)

#define loshader_event_line_drawer_clear(drawer, reserve)  \
    loshader_text_drawer_clear(&(drawer)->super, reserve)

#define loshader_event_line_drawer_add_block(drawer, block)  \
    loshader_text_drawer_add_block(&(drawer)->super, block)

#define loshader_event_line_drawer_draw(drawer)  \
    loshader_text_drawer_draw(&(drawer)->super)
