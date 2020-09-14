#include "./atlas.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <GL/glew.h>

#include "util/container/array.h"
#include "util/math/algorithm.h"
#include "util/memory/memory.h"

#include "./misc.h"
#include "./texture.h"

struct gleasy_atlas_t {
  int32_t width;
  int32_t height;

  int32_t consumed_y;
  int32_t consumed_x;

  int32_t line_height;

  gleasy_texture_2d_t tex;

  CONTAINER_ARRAY uint8_t* resize_buffer;
};

static void gleasy_atlas_resize_bitmap_(
    gleasy_atlas_t*              atlas,
    gleasy_atlas_bitmap_t*       out,
    const gleasy_atlas_bitmap_t* in) {
  assert(atlas != NULL);
  assert(out   != NULL);
  assert(in    != NULL);

  *out = (typeof(*out)) {
    .width  = math_int32_next_power2(MATH_MAX(in->width, 4)),
    .height = math_int32_next_power2(MATH_MAX(in->height, 4)),
    .format = in->format,
    .type   = in->type,
    .buffer = in->buffer,
  };
  if (out->width == in->width && out->height == in->height) return;

  const size_t type = GLEASY_GET_BYTE_SIZE_OF_TYPE(out->type);
  const size_t fmt  = GLEASY_GET_CHANNELS_OF_TEXTURE_FORMAT(out->format);

  const size_t pixel = type*fmt;
  assert(pixel > 0);

  container_array_resize(atlas->resize_buffer, out->width*out->height*pixel);
  out->buffer = atlas->resize_buffer;

  const int32_t ymax = out->height * pixel;
  const int32_t xmax = out->width  * pixel;

  const uint8_t* src = in->buffer;
  uint8_t*       dst = atlas->resize_buffer;
  for (int32_t y = 0; y < ymax; ++y) {
    for (int32_t x = 0; x < xmax; ++x) {
      if (x < in->width && y < in->height) {
        *dst = *(src++);
      } else {
        *dst = 0;
      }
      ++dst;
    }
  }
}

static bool gleasy_atlas_allocate_area_(
    gleasy_atlas_t* atlas,
    int32_t*        x,
    int32_t*        y,
    int32_t         width,
    int32_t         height,
    int32_t         actual_width,
    int32_t         actual_height) {
  assert(atlas != NULL);
  assert(x     != NULL);
  assert(y     != NULL);
  assert(width  > 0);
  assert(height > 0);
  assert(actual_width  > 0);
  assert(actual_height > 0);

  if (atlas->consumed_x + actual_width > atlas->width) {
    atlas->consumed_x  = 0;
    atlas->consumed_y += atlas->line_height+1;
    atlas->line_height = 0;
  }
  if (atlas->consumed_y + actual_height > atlas->height) {
    return false;
  }

  *x = atlas->consumed_x;
  *y = atlas->consumed_y;

  atlas->consumed_x += width+1;
  atlas->line_height = MATH_MAX(atlas->line_height, height);
  return true;
}

gleasy_atlas_t* gleasy_atlas_new(
    GLenum format, int32_t width, int32_t height, bool aa) {
  assert(width  > 0);
  assert(height > 0);

  gleasy_atlas_t* atlas = memory_new(sizeof(*atlas));
  *atlas = (typeof(*atlas)) {
    .width  = math_int32_next_power2(width),
    .height = math_int32_next_power2(height),
  };

  glGenTextures(1, &atlas->tex);
  glBindTexture(GL_TEXTURE_2D, atlas->tex);

  const GLenum filter = aa? GL_LINEAR: GL_NEAREST;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

  glTexImage2D(GL_TEXTURE_2D, 0, format,
      atlas->width, atlas->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  assert(glGetError() == GL_NO_ERROR);

  container_array_reserve(atlas->resize_buffer, width*height*4);
  return atlas;
}

void gleasy_atlas_delete(gleasy_atlas_t* atlas) {
  if (atlas == NULL) return;

  glDeleteTextures(1, &atlas->tex);

  container_array_delete(atlas->resize_buffer);

  memory_delete(atlas);
}

void gleasy_atlas_clear(gleasy_atlas_t* atlas) {
  assert(atlas != NULL);

  atlas->consumed_y = 0;
  atlas->consumed_x = 0;

  atlas->line_height = 0;
}

bool gleasy_atlas_add(
    gleasy_atlas_t*              atlas,
    gleasy_atlas_geometry_t*     geo,
    const gleasy_atlas_bitmap_t* bitmap) {
  assert(atlas  != NULL);
  assert(geo    != NULL);
  assert(bitmap != NULL);

  gleasy_atlas_bitmap_t resized;
  gleasy_atlas_resize_bitmap_(atlas, &resized, bitmap);

  int32_t x, y;
  if (!gleasy_atlas_allocate_area_(atlas,
        &x, &y, bitmap->width, bitmap->height, resized.width, resized.height)) {
    return false;
  }

  *geo = (typeof(*geo)) {
    .left   = x*1.0f / atlas->width,
    .right  = (x+bitmap->width)*1.0f / atlas->width,
    .top    = y*1.0f / atlas->height,
    .bottom = (y+bitmap->height)*1.0f / atlas->height,
  };

  glBindTexture(GL_TEXTURE_2D, atlas->tex);
  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y,
      resized.width,
      resized.height,
      resized.format,
      resized.type,
      resized.buffer);
  assert(glGetError() == GL_NO_ERROR);

  return true;
}

gleasy_texture_2d_t gleasy_atlas_get_texture(const gleasy_atlas_t* atlas) {
  assert(atlas != NULL);

  return atlas->tex;
}
