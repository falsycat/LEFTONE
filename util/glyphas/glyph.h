#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
  int32_t  width;
  int32_t  height;
  uint8_t* buffer;  /* gray-scale bitmap */
} glyphas_glyph_bitmap_t;

typedef struct {
  int32_t bear_x;
  int32_t bear_y;
  int32_t advance;
} glyphas_glyph_metrics_t;

typedef struct {
  glyphas_glyph_bitmap_t  bitmap;
  glyphas_glyph_metrics_t vmetrics;
  glyphas_glyph_metrics_t hmetrics;
} glyphas_glyph_t;

/* The glyph's lifetime must be shorter than the slot. */
bool
glyphas_glyph_reset_from_ft_glyph_slot(
    glyphas_glyph_t* glyph,
    FT_GlyphSlot     slot
);
