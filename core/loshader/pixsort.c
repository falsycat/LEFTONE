#include "./pixsort.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "util/gleasy/framebuffer.h"
#include "util/gleasy/program.h"

#include "./single.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/pixsort.vshader.h"
#include "core/loshader/anysrc/pixsort.fshader.h"

#define UNIFORM_SRC_       0
#define UNIFORM_INTENSITY_ 1

#define PRIMITIVE_COUNT_ 6

void loshader_pixsort_drawer_initialize(
    loshader_pixsort_drawer_t*  drawer,
    const loshader_uniblock_t*  uniblock,
    const gleasy_framebuffer_t* fb) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);
  assert(fb       != NULL);

  *drawer = (typeof(*drawer)) {
    .fb = fb,
  };

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,   sizeof(loshader_header_shader_),
      loshader_pixsort_vshader_, sizeof(loshader_pixsort_vshader_),
      loshader_pixsort_fshader_, sizeof(loshader_pixsort_fshader_));

  loshader_single_drawer_initialize(&drawer->super, prog, uniblock, 0);
}

void loshader_pixsort_drawer_draw(
    const loshader_pixsort_drawer_t* drawer) {
  assert(drawer != NULL);

  glUseProgram(drawer->super.prog);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, drawer->fb->colorbuf);

  glUniform1i(UNIFORM_SRC_, 0);
  glUniform1f(UNIFORM_INTENSITY_, drawer->intensity);

  loshader_single_drawer_draw_without_use_program(
      &drawer->super, PRIMITIVE_COUNT_);
}

bool loshader_pixsort_drawer_is_skippable(
    const loshader_pixsort_drawer_t* drawer) {
  assert(drawer != NULL);

  return drawer->intensity == 0;
}
