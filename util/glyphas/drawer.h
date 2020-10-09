#pragma once

#include "util/gleasy/buffer.h"
#include "util/gleasy/texture.h"
#include "util/gleasy/program.h"

#include "./block.h"

typedef struct {
  gleasy_texture_2d_t tex;

  GLuint vao;

  gleasy_buffer_array_t instances;
  size_t                instances_reserved;
  size_t                instances_length;
} glyphas_drawer_t;

gleasy_program_t
glyphas_drawer_create_default_program(
    void
);

void
glyphas_drawer_initialize(
    glyphas_drawer_t*   drawer,
    gleasy_texture_2d_t tex
);

void
glyphas_drawer_deinitialize(
    glyphas_drawer_t* drawer
);

void
glyphas_drawer_clear(
    glyphas_drawer_t* drawer,
    size_t            reserve
);

void
glyphas_drawer_add_block(
    glyphas_drawer_t*      drawer,
    const glyphas_block_t* block
        /* The drawer doesn't hold the reference. */
);

void
glyphas_drawer_add_block_item(
    glyphas_drawer_t*           drawer,
    const glyphas_block_item_t* item
        /* The drawer doesn't hold the reference. */
);

/* call glUseProgram before calling */
void
glyphas_drawer_draw(
    const glyphas_drawer_t* drawer
);
