#pragma once

#include "util/glyphas/context.h"
#include "util/glyphas/face.h"

typedef struct {
  glyphas_context_t glyphas;

  glyphas_face_t sans;
  glyphas_face_t serif;
} loresource_font_t;

void
loresource_font_initialize(
    loresource_font_t* font
);

void
loresource_font_deinitialize(
    loresource_font_t* font
);
