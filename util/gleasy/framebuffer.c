#include "./framebuffer.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "./texture.h"

void gleasy_framebuffer_initialize(
    gleasy_framebuffer_t* fb, int32_t width, int32_t height, int32_t samples) {
  assert(fb != NULL);
  assert(width   > 0);
  assert(height  > 0);
  assert(0 < samples && samples < GL_MAX_SAMPLES);

  *fb = (typeof(*fb)) {
    .width  = width,
    .height = height,
  };

  glGenRenderbuffers(1, &fb->colorbuf_msaa);
  glBindRenderbuffer(GL_RENDERBUFFER, fb->colorbuf_msaa);
  glRenderbufferStorageMultisample(
      GL_RENDERBUFFER, samples, GL_RGBA, width, height);
  assert(glGetError() == GL_NO_ERROR);

  glGenTextures(1, &fb->colorbuf);
  glBindTexture(GL_TEXTURE_2D, fb->colorbuf);
  glTexImage2D(GL_TEXTURE_2D,
      0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  assert(glGetError() == GL_NO_ERROR);

  GLuint fbo[2];
  glGenFramebuffers(2, fbo);
  fb->id_msaa = fbo[0];
  fb->id      = fbo[1];

  glBindFramebuffer(GL_FRAMEBUFFER, fb->id_msaa);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fb->colorbuf_msaa);
  assert(glGetError() == GL_NO_ERROR);

  glBindFramebuffer(GL_FRAMEBUFFER, fb->id);
  glFramebufferTexture2D(GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->colorbuf, 0);
  assert(glGetError() == GL_NO_ERROR);
}

void gleasy_framebuffer_deinitialize(gleasy_framebuffer_t* fb) {
  assert(fb != NULL);

  GLuint fbo[] = {fb->id_msaa, fb->id};
  glDeleteFramebuffers(2, fbo);

  glDeleteTextures(1, &fb->colorbuf);

  glDeleteRenderbuffers(1, &fb->colorbuf_msaa);
}

void gleasy_framebuffer_bind(const gleasy_framebuffer_t* fb) {
  assert(fb != NULL);

  glBindFramebuffer(GL_FRAMEBUFFER, fb->id_msaa);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "gleasy: framebuffer is in invalid state\n");
    abort();
  }
}

void gleasy_framebuffer_flush(const gleasy_framebuffer_t* fb) {
  assert(fb != NULL);

  gleasy_framebuffer_flush_to_other(fb, fb->id);
}

void gleasy_framebuffer_flush_to_other(const gleasy_framebuffer_t* fb, GLuint id) {
  assert(fb != NULL);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fb->id_msaa);
  glBlitFramebuffer(
      0, 0, fb->width, fb->height,
      0, 0, fb->width, fb->height,
      GL_COLOR_BUFFER_BIT,
      GL_NEAREST);
  assert(glGetError() == GL_NO_ERROR);
}
