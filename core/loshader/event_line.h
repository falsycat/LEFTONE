#pragma once

#include <stddef.h>

#include "util/gleasy/program.h"
#include "util/gleasy/texture.h"
#include "util/glyphas/block.h"
#include "util/glyphas/drawer.h"

#include "./uniblock.h"

typedef gleasy_program_t loshader_event_line_program_t;

typedef struct {
  /* injected deps */
  const loshader_event_line_program_t* prog;
  const loshader_uniblock_t*           uniblock;

  gleasy_texture_2d_t tex;

  /* owned objects */
  glyphas_drawer_t* glyphas;
} loshader_event_line_drawer_t;

void
loshader_event_line_program_initialize(
    loshader_event_line_program_t* prog
);

void
loshader_event_line_program_deinitialize(
    loshader_event_line_program_t* prog
);

void
loshader_event_line_drawer_initialize(
    loshader_event_line_drawer_t*        drawer,
    const loshader_event_line_program_t* prog,
    const loshader_uniblock_t*          uniblock,
    gleasy_texture_2d_t                 tex
);

void
loshader_event_line_drawer_deinitialize(
    loshader_event_line_drawer_t* drawer
);

void
loshader_event_line_drawer_clear(
    loshader_event_line_drawer_t* drawer,
    size_t                        reserve
);

void
loshader_event_line_drawer_add_block(
    loshader_event_line_drawer_t* drawer,
    const glyphas_block_t*        block
);

void
loshader_event_line_drawer_draw(
    const loshader_event_line_drawer_t* drawer
);
