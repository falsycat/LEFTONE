#pragma once

#include <stdint.h>

#include <GL/glew.h>

#include "./texture.h"

typedef struct {
  int32_t width;
  int32_t height;

  GLuint id_msaa;
  GLuint colorbuf_msaa;

  GLuint id;
  gleasy_texture_2d_t colorbuf;
} gleasy_framebuffer_t;

void
gleasy_framebuffer_initialize(
    gleasy_framebuffer_t* fb,
    int32_t               width,
    int32_t               height,
    int32_t               samples
);

void
gleasy_framebuffer_deinitialize(
    gleasy_framebuffer_t* fb
);

void
gleasy_framebuffer_bind(
    const gleasy_framebuffer_t* fb
);

void
gleasy_framebuffer_flush(
    const gleasy_framebuffer_t* fb
);

void
gleasy_framebuffer_flush_to_other(
    const gleasy_framebuffer_t* fb,
    GLuint                      id
);
