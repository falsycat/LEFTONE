#include "./cache.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <GL/glew.h>

#include "util/container/array.h"
#include "util/gleasy/atlas.h"
#include "util/memory/memory.h"

#include "./face.h"

struct glyphas_cache_t {
  gleasy_atlas_t* atlas;
  glyphas_face_t* face;

  int32_t char_width;
  int32_t char_height;

  /* TODO(catfoot): linear search isn't efficient. :( */
  CONTAINER_ARRAY glyphas_cache_glyph_t* items;
};

glyphas_cache_t* glyphas_cache_new(
    gleasy_atlas_t* atlas,
    glyphas_face_t* face,
    int32_t         char_width,
    int32_t         char_height) {
  assert(atlas       != NULL);
  assert(face        != NULL);
  assert(char_width  > 0);
  assert(char_height > 0);

  glyphas_cache_t* cache = memory_new(sizeof(*cache));
  *cache = (typeof(*cache)) {
    .atlas       = atlas,
    .face        = face,
    .char_width  = char_width,
    .char_height = char_height,
  };
  return cache;
}

void glyphas_cache_delete(glyphas_cache_t* cache) {
  if (cache == NULL) return;

  container_array_delete(cache->items);

  memory_delete(cache);
}

const glyphas_cache_glyph_t* glyphas_cache_add_glyph(
    glyphas_cache_t* cache, uint32_t unicode) {
  assert(cache != NULL);

  const glyphas_cache_glyph_t* found =
      glyphas_cache_lookup_glyph(cache, unicode);
  if (found != NULL) return found;

  if (!glyphas_face_set_pixel_size(
        cache->face, cache->char_width, cache->char_height)) {
    return NULL;
  }
  if (!glyphas_face_render_glyph(cache->face, unicode)) {
    return NULL;
  }

  gleasy_atlas_geometry_t geo = {0};

  const glyphas_glyph_t* rendered = &cache->face->glyph;
  if (rendered->bitmap.width > 0) {
    const gleasy_atlas_bitmap_t bmp = {
      .width  = rendered->bitmap.width,
      .height = rendered->bitmap.height,
      .buffer = rendered->bitmap.buffer,
      .format = GL_RED,
      .type   = GL_UNSIGNED_BYTE,
    };
    if (!gleasy_atlas_add(cache->atlas, &geo, &bmp)) {
      return NULL;
    }
  }

  const size_t index = container_array_get_length(cache->items);
  container_array_insert(cache->items, index);

  glyphas_cache_glyph_t* g = &cache->items[index];
  *g = (typeof(*g)) {
    .unicode  = unicode,
    .width    = rendered->bitmap.width,
    .height   = rendered->bitmap.height,
    .geometry = geo,
    .hmetrics = rendered->hmetrics,
    .vmetrics = rendered->vmetrics,
  };
  return g;
}

const glyphas_cache_glyph_t* glyphas_cache_lookup_glyph(
    const glyphas_cache_t* cache, uint32_t unicode) {
  assert(cache != NULL);

  const size_t len = container_array_get_length(cache->items);
  for (size_t i = 0; i < len; ++i) {
    const glyphas_cache_glyph_t* g = &cache->items[i];
    if (g->unicode == unicode) return g;
  }
  return NULL;
}
