#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "./context.h"
#include "./glyph.h"

typedef struct {
  FT_Face ft;

  glyphas_glyph_t glyph;
} glyphas_face_t;

void
glyphas_face_initialize_from_file(
    glyphas_face_t*          face,
    const glyphas_context_t* ctx,
    const char*              path,
    size_t                   index
);

void
glyphas_face_initialize_from_buffer(
    glyphas_face_t*          face,
    const glyphas_context_t* ctx,
    const void*              data,
    size_t                   length,
    size_t                   index
);

void
glyphas_face_deinitialize(
    glyphas_face_t* face
);

bool
glyphas_face_set_pixel_size(
    glyphas_face_t* face,
    int32_t         width,
    int32_t         height
);

/* Set sizes before rendering glyphs. */
bool
glyphas_face_render_glyph(
    glyphas_face_t* face,
    uint64_t        unicode
);
