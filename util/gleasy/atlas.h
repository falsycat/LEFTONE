#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <GL/glew.h>

#include "./texture.h"

struct gleasy_atlas_t;
typedef struct gleasy_atlas_t gleasy_atlas_t;

typedef struct {
  float left;
  float right;
  float top;
  float bottom;
} gleasy_atlas_geometry_t;

typedef struct {
  int32_t        width;
  int32_t        height;
  GLenum         format;
  GLenum         type;
  const uint8_t* buffer;
} gleasy_atlas_bitmap_t;

gleasy_atlas_t*
gleasy_atlas_new(
    GLenum  format,
    int32_t width,
    int32_t height,
    bool    aa
);

void
gleasy_atlas_delete(
    gleasy_atlas_t* atlas
);

void
gleasy_atlas_clear(
    gleasy_atlas_t* atlas
);

bool
gleasy_atlas_add(
    gleasy_atlas_t*              atlas,
    gleasy_atlas_geometry_t*     geo,
    const gleasy_atlas_bitmap_t* bitmap
);

gleasy_texture_2d_t
gleasy_atlas_get_texture(
    const gleasy_atlas_t* atlas
);
