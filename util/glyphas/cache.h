#pragma once

#include <stdint.h>

#include "util/gleasy/atlas.h"

#include "./face.h"
#include "./glyph.h"

typedef struct {
  uint32_t unicode;

  int32_t width;
  int32_t height;

  gleasy_atlas_geometry_t geometry;
  glyphas_glyph_metrics_t hmetrics;
  glyphas_glyph_metrics_t vmetrics;
} glyphas_cache_glyph_t;

struct glyphas_cache_t;
typedef struct glyphas_cache_t glyphas_cache_t;

glyphas_cache_t*
glyphas_cache_new(
    gleasy_atlas_t* atlas,
    glyphas_face_t* face,
    int32_t         char_width,
    int32_t         char_height
);

void
glyphas_cache_delete(
    glyphas_cache_t* cache
);

const glyphas_cache_glyph_t*  /* NULLABLE */
glyphas_cache_add_glyph(
    glyphas_cache_t* cache,
    uint32_t         unicode
);

const glyphas_cache_glyph_t*  /* NULLABLE */
glyphas_cache_lookup_glyph(
    const glyphas_cache_t* cache,
    uint32_t               unicode
);
