#pragma once

#include <stddef.h>

#include "util/gleasy/program.h"
#include "util/gleasy/texture.h"
#include "util/glyphas/block.h"
#include "util/glyphas/drawer.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_hud_text_program_t;

typedef struct {
  /* injected deps */
  const loshader_hud_text_program_t* prog;
  const loshader_uniblock_t*         uniblock;

  gleasy_texture_2d_t tex;

  /* owned objects */
  glyphas_drawer_t* glyphas;

  /* public params */
  float alpha;
} loshader_hud_text_drawer_t;

void
loshader_hud_text_program_initialize(
    loshader_hud_text_program_t* prog
);

void
loshader_hud_text_program_deinitialize(
    loshader_hud_text_program_t* prog
);

void
loshader_hud_text_drawer_initialize(
    loshader_hud_text_drawer_t*        drawer,
    const loshader_hud_text_program_t* prog,
    const loshader_uniblock_t*         uniblock,
    gleasy_texture_2d_t                tex
);

void
loshader_hud_text_drawer_deinitialize(
    loshader_hud_text_drawer_t* drawer
);

void
loshader_hud_text_drawer_clear(
    loshader_hud_text_drawer_t* drawer,
    size_t                      reserve
);

void
loshader_hud_text_drawer_add_block(
    loshader_hud_text_drawer_t* drawer,
    const glyphas_block_t*      block
);

void
loshader_hud_text_drawer_draw(
    const loshader_hud_text_drawer_t* drawer
);
