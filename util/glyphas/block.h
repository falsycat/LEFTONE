#pragma once

#include <stddef.h>
#include <stdint.h>

#include "util/math/vector.h"
#include "util/gleasy/atlas.h"

#include "./aligner.h"
#include "./cache.h"

struct glyphas_block_t;
typedef struct glyphas_block_t glyphas_block_t;

typedef struct {
  vec2_t                  pos;
  vec2_t                  size;
  vec4_t                  color;
  gleasy_atlas_geometry_t uv;
} glyphas_block_item_t;

glyphas_block_t*
glyphas_block_new(
    glyphas_aligner_direction_t dir,
    int32_t                     lineheight,
    int32_t                     maxpos,
    size_t                      reserve
);

void
glyphas_block_delete(
    glyphas_block_t* block
);

void
glyphas_block_clear(
    glyphas_block_t* block
);

void
glyphas_block_add_character(
    glyphas_block_t*             block,
    const glyphas_cache_glyph_t* glyph,
    const vec4_t*                color
);

void
glyphas_block_add_characters(
    glyphas_block_t* block,
    glyphas_cache_t* cache,
    const vec4_t*    color,
    const char*      str,
    size_t           len
);

void
glyphas_block_break_line(
    glyphas_block_t* block
);

void
glyphas_block_scale(
    glyphas_block_t* block,
    const vec2_t*    translation
);

void
glyphas_block_translate(
    glyphas_block_t* block,
    const vec2_t*    translation
);

void
glyphas_block_set_origin(
    glyphas_block_t* block,
    const vec2_t*    r
);

void
glyphas_block_set_alpha(
    glyphas_block_t* block,
    float            a
);

void
glyphas_block_make_glitched(
    glyphas_block_t* block,
    uint64_t         seed
);

void
glyphas_block_calculate_geometry(
    const glyphas_block_t* block,
    vec2_t*                offset,
    vec2_t*                size
);

const glyphas_block_item_t*
glyphas_block_get_items(
    const glyphas_block_t* block,
    size_t*                len
);
