#pragma once

#include "util/gleasy/texture.h"
#include "util/gleasy/program.h"

#include "./block.h"

struct glyphas_drawer_t;
typedef struct glyphas_drawer_t glyphas_drawer_t;

gleasy_program_t
glyphas_drawer_create_default_program(
    void
);

glyphas_drawer_t*
glyphas_drawer_new(
    void
);

void
glyphas_drawer_delete(
    glyphas_drawer_t* drawer
);

void
glyphas_drawer_clear(
    glyphas_drawer_t*   drawer,
    gleasy_texture_2d_t tex,
    size_t              reserve
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

/* bind the program before drawing */
void
glyphas_drawer_draw(
    const glyphas_drawer_t* drawer
);
