#include "./block.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/chaos/xorshift.h"
#include "util/conv/charcode.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/gleasy/atlas.h"
#include "util/memory/memory.h"

#include "./cache.h"

struct glyphas_block_t {
  glyphas_aligner_t aligner;

  size_t               reserve;
  size_t               length;
  glyphas_block_item_t items[1];
};

glyphas_block_t* glyphas_block_new(
    glyphas_aligner_direction_t dir,
    int32_t                     lineheight,
    int32_t                     maxpos,
    size_t                      reserve) {
  assert(maxpos  > 0);
  assert(reserve > 0);

  glyphas_block_t* block =
      memory_new(sizeof(*block) + (reserve-1)*sizeof(block->items[0]));
  *block = (typeof(*block)) {
    .reserve = reserve,
  };

  glyphas_aligner_initialize(&block->aligner, dir, lineheight, maxpos);

  return block;
}

void glyphas_block_delete(glyphas_block_t* block) {
  if (block == NULL) return;

  glyphas_aligner_deinitialize(&block->aligner);

  memory_delete(block);
}

void glyphas_block_clear(glyphas_block_t* block) {
  assert(block != NULL);

  glyphas_aligner_reset(&block->aligner);
  block->length = 0;
}

void glyphas_block_add_character(
    glyphas_block_t*             block,
    const glyphas_cache_glyph_t* glyph,
    const vec4_t*                color) {
  assert(block != NULL);
  assert(glyph != NULL);
  assert(vec4_valid(color));

  if (block->length >= block->reserve) {
    fprintf(stderr, "glyphas block overflow\n");
    abort();
  }

  int32_t x, y;
  glyphas_aligner_push_character(&block->aligner, &x, &y, glyph);
  if (glyph->geometry.left >= glyph->geometry.right) return;

  block->items[block->length++] = (glyphas_block_item_t) {
    .pos     = vec2(x, y),
    .size    = vec2(glyph->width, glyph->height),
    .color   = *color,
    .uv      = glyph->geometry,
  };
}

void glyphas_block_add_characters(
    glyphas_block_t* block,
    glyphas_cache_t* cache,
    const vec4_t*    color,
    const char*      str,
    size_t           len) {
  assert(block != NULL);
  assert(cache != NULL);
  assert(vec4_valid(color));

  const char* end = str + len;
  while (str < end) {
    uint32_t c;
    const size_t consumed = conv_charcode_utf8_to_utf32(&c, str, end-str);
    str += MATH_MAX(consumed, 1);
    if (consumed == 0) continue;

    if (c == '\n') {
      glyphas_block_break_line(block);
      continue;
    }

    const glyphas_cache_glyph_t* g = glyphas_cache_add_glyph(cache, c);
    if (g == NULL) continue;

    glyphas_block_add_character(block, g, color);
  }
}

void glyphas_block_break_line(glyphas_block_t* block) {
  assert(block != NULL);

  glyphas_aligner_break_line(&block->aligner);
}

void glyphas_block_scale(glyphas_block_t* block, const vec2_t* s) {
  assert(block != NULL);
  assert(vec2_valid(s));

  for (size_t i = 0; i < block->length; ++i) {
    vec2_t* pos = &block->items[i].pos;
    vec2_t* sz  = &block->items[i].size;

    pos->x *= s->x;
    pos->y *= s->y;
    sz->x  *= s->x;
    sz->y  *= s->y;
  }
}

void glyphas_block_translate(glyphas_block_t* block, const vec2_t* v) {
  assert(block != NULL);
  assert(vec2_valid(v));

  for (size_t i = 0; i < block->length; ++i) {
    vec2_addeq(&block->items[i].pos, v);
  }
}

void glyphas_block_set_origin(glyphas_block_t* block, const vec2_t* r) {
  assert(block !=NULL);
  assert(vec2_valid(r));

  vec2_t offset, size;
  glyphas_block_calculate_geometry(block, &offset, &size);

  size.x *= r->x;
  size.y *= r->y;

  vec2_addeq(&offset, &size);
  vec2_muleq(&offset, -1);
  glyphas_block_translate(block, &offset);
}

void glyphas_block_set_alpha(glyphas_block_t* block, float a) {
  assert(block != NULL);

  for (size_t i = 0; i < block->length; ++i) {
    block->items[i].color.w = a;
  }
}

void glyphas_block_make_glitched(glyphas_block_t* block, uint64_t seed) {
  assert(block != NULL);

# define randf() (seed = chaos_xorshift(seed), seed%1000/1000.f)

  for (size_t i = 0; i < block->length; ++i) {
    const vec2_t* sz = &block->items[i].size;

    const vec2_t shift = vec2((randf()-.5f)*sz->x*.5f, (randf()-.5f)*sz->y*.5f);
    const float  scale = randf()*.4f + .8f;

    vec2_addeq(&block->items[i].pos,  &shift);
    vec2_muleq(&block->items[i].size, scale);
  }

# undef randf
}

void glyphas_block_calculate_geometry(
    const glyphas_block_t* block, vec2_t* offset, vec2_t* size) {
  assert(block  != NULL);
  assert(offset != NULL);
  assert(size   != NULL);

  vec2_t lb = vec2(0, 0), rt = vec2(0, 0);
  for (size_t i = 0; i < block->length; ++i) {
    const glyphas_block_item_t* item = &block->items[i];

    const float l = item->pos.x;
    const float r = l + item->size.x;
    const float t = item->pos.y;
    const float b = t - item->size.y;

    lb.x = MATH_MIN(lb.x, l);
    lb.y = MATH_MIN(lb.y, b);
    rt.x = MATH_MAX(rt.x, r);
    rt.y = MATH_MAX(rt.y, t);
  }
  *offset = vec2(lb.x, rt.y);
  vec2_sub(size, &rt, &lb);
}

const glyphas_block_item_t* glyphas_block_get_items(
    const glyphas_block_t* block, size_t* len) {
  assert(block != NULL);
  assert(len   != NULL);

  *len = block->length;
  return block->items;
}
