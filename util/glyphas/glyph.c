#include "./glyph.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

bool glyphas_glyph_reset_from_ft_glyph_slot(
    glyphas_glyph_t* glyph, FT_GlyphSlot slot) {
  assert(glyph != NULL);
  assert(slot  != NULL);

  if (slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) return false;

  *glyph = (typeof(*glyph)) {
    .bitmap = {
      .width  = slot->bitmap.width,
      .height = slot->bitmap.rows,
      .buffer = slot->bitmap.buffer,
    },
    .hmetrics = {
      .bear_x  = slot->metrics.horiBearingX/64,
      .bear_y  = slot->metrics.horiBearingY/64,
      .advance = slot->metrics.horiAdvance /64,
    },
    .vmetrics = {
      .bear_x  = slot->metrics.vertBearingX/64,
      .bear_y  = slot->metrics.vertBearingY/64,
      .advance = slot->metrics.vertAdvance /64,
    },
  };
  return true;
}
