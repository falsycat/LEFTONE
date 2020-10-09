#pragma once

#include "util/gleasy/program.h"
#include "util/gleasy/texture.h"
#include "util/glyphas/drawer.h"

#include "./uniblock.h"

typedef struct {
  glyphas_drawer_t           super;
  gleasy_program_t           prog;
  const loshader_uniblock_t* uniblock;

  float alpha;
} loshader_text_drawer_t;

void
loshader_text_drawer_initialize(
    loshader_text_drawer_t*    drawer,
    gleasy_program_t           prog,  /* OWNERSHIP */
    const loshader_uniblock_t* uniblock,
    gleasy_texture_2d_t        tex
);

void
loshader_text_drawer_deinitialize(
    loshader_text_drawer_t* drawer
);

#define loshader_text_drawer_clear(drawer, reserve)  \
    glyphas_drawer_clear(&(drawer)->super, reserve)

#define loshader_text_drawer_add_block(drawer, block)  \
    glyphas_drawer_add_block(&(drawer)->super, block)

void
loshader_text_drawer_draw(
    const loshader_text_drawer_t* drawer
);

void
loshader_text_drawer_draw_without_use_program(
    const loshader_text_drawer_t* drawer
);
