#include "./aligner.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/gleasy/atlas.h"
#include "util/math/algorithm.h"

#include "./cache.h"

void glyphas_aligner_initialize(
    glyphas_aligner_t*          aligner,
    glyphas_aligner_direction_t dir,
    int32_t                     lineheight,
    int32_t                     maxpos) {
  assert(aligner != NULL);
  assert(maxpos > 0);

  *aligner = (typeof(*aligner)) {
    .dir        = dir,
    .lineheight = lineheight,
    .maxpos     = maxpos,
    .line       = lineheight,
  };
}

void glyphas_aligner_deinitialize(glyphas_aligner_t* aligner) {
  assert(aligner != NULL);

}

void glyphas_aligner_reset(glyphas_aligner_t* aligner) {
  assert(aligner != NULL);

  aligner->line = aligner->lineheight;
  aligner->pos  = 0;
}

void glyphas_aligner_push_character(
    glyphas_aligner_t*           aligner,
    int32_t*                     x,
    int32_t*                     y,
    const glyphas_cache_glyph_t* g) {
  assert(aligner != NULL);
  assert(x       != NULL);
  assert(y       != NULL);
  assert(g       != NULL);

  switch (aligner->dir) {
  case GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL:
    if (aligner->pos + g->hmetrics.advance > aligner->maxpos) {
      glyphas_aligner_break_line(aligner);
    }
    *x = aligner->pos + g->hmetrics.bear_x;
    *y = g->hmetrics.bear_y + aligner->line;
    aligner->pos += g->hmetrics.advance;
    break;
  case GLYPHAS_ALIGNER_DIRECTION_VERTICAL:
    if (aligner->pos - g->vmetrics.advance < -aligner->maxpos) {
      glyphas_aligner_break_line(aligner);
    }
    *x = aligner->pos - g->vmetrics.bear_x + aligner->line;
    *y = -g->vmetrics.bear_y;
    aligner->pos -= g->vmetrics.advance;
    break;
  default:
    assert(false);
  }
}

void glyphas_aligner_break_line(glyphas_aligner_t* aligner) {
  assert(aligner != NULL);

  aligner->pos   = 0;
  aligner->line += aligner->lineheight;
}
