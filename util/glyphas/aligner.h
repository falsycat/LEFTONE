#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "./cache.h"

typedef enum {
  GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
  GLYPHAS_ALIGNER_DIRECTION_VERTICAL,
} glyphas_aligner_direction_t;

typedef struct {
  glyphas_aligner_direction_t dir;
  int32_t lineheight;
  int32_t maxpos;

  int32_t pos;
  int32_t line;
} glyphas_aligner_t;

void
glyphas_aligner_initialize(
    glyphas_aligner_t*          aligner,
    glyphas_aligner_direction_t dir,
    int32_t                     lineheight,
    int32_t                     maxpos
);

void
glyphas_aligner_deinitialize(
    glyphas_aligner_t* aligner
);

void
glyphas_aligner_reset(
    glyphas_aligner_t* aligner
);

void
glyphas_aligner_push_character(
    glyphas_aligner_t*           aligner,
    int32_t*                     x,
    int32_t*                     y,
    const glyphas_cache_glyph_t* g
);

void
glyphas_aligner_break_line(
    glyphas_aligner_t* aligner
);
