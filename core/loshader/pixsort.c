#include "./pixsort.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "util/gleasy/framebuffer.h"
#include "util/gleasy/program.h"
#include "util/math/algorithm.h"
#include "util/memory/memory.h"

#include "./uniblock.h"

/* resources */
#include "anysrc/header.shader.h"
#include "anysrc/pixsort.vshader.h"
#include "anysrc/pixsort.fshader.h"

#define LOSHADER_PIXSORT_UNIFORM_SRC       0
#define LOSHADER_PIXSORT_UNIFORM_INTENSITY 1

#define LOSHADER_PIXSORT_UNIBLOCK_INDEX 0

struct loshader_pixsort_drawer_t {
  const loshader_pixsort_program_t* prog;
  const loshader_uniblock_t*           uniblock;

  const gleasy_framebuffer_t* fb;

  float intensity;
};

void loshader_pixsort_program_initialize(
    loshader_pixsort_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,   sizeof(loshader_header_shader_),
      loshader_pixsort_vshader_, sizeof(loshader_pixsort_vshader_),
      loshader_pixsort_fshader_, sizeof(loshader_pixsort_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, uniblock, LOSHADER_PIXSORT_UNIBLOCK_INDEX);
}

void loshader_pixsort_program_deinitialize(
    loshader_pixsort_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_pixsort_drawer_t* loshader_pixsort_drawer_new(
    const loshader_pixsort_program_t* prog,
    const loshader_uniblock_t*        uniblock,
    const gleasy_framebuffer_t*       fb) {
  assert(prog     != NULL);
  assert(uniblock != NULL);
  assert(fb       != NULL);

  loshader_pixsort_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
    .fb       = fb,
  };
  return drawer;
}

void loshader_pixsort_drawer_delete(loshader_pixsort_drawer_t* drawer) {
  if (drawer == NULL) return;

  memory_delete(drawer);
}

void loshader_pixsort_drawer_set_intensity(
    loshader_pixsort_drawer_t* drawer,
    float                      intensity) {
  assert(drawer != NULL);
  assert(MATH_FLOAT_VALID(intensity));

  drawer->intensity = intensity;
}

void loshader_pixsort_drawer_draw(
    const loshader_pixsort_drawer_t* drawer) {
  assert(drawer != NULL);

  glUseProgram(*drawer->prog);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, drawer->fb->colorbuf);
  glUniform1i(LOSHADER_PIXSORT_UNIFORM_SRC, 0);

  glUniform1f(LOSHADER_PIXSORT_UNIFORM_INTENSITY, drawer->intensity);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool loshader_pixsort_drawer_is_skippable(
    const loshader_pixsort_drawer_t* drawer) {
  assert(drawer != NULL);

  return drawer->intensity == 0;
}
