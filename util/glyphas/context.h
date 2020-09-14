#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
  FT_Library ft;
} glyphas_context_t;

void
glyphas_context_initialize(
    glyphas_context_t* ctx
);

void
glyphas_context_deinitialize(
    glyphas_context_t* ctx
);
