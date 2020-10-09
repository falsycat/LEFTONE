#include "./menu_background.h"

#include <assert.h>
#include <stddef.h>

#include "util/gleasy/program.h"
#include "util/math/algorithm.h"

#include "./single.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/menu_background.vshader.h"
#include "core/loshader/anysrc/menu_background.fshader.h"

#define UNIFORM_ALPHA_ 0

#define PRIMITIVE_COUNT_ 6

void loshader_menu_background_drawer_initialize(
    loshader_menu_background_drawer_t* drawer,
    const loshader_uniblock_t*         uniblock) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,
      sizeof(loshader_header_shader_),
      loshader_menu_background_vshader_,
      sizeof(loshader_menu_background_vshader_),
      loshader_menu_background_fshader_,
      sizeof(loshader_menu_background_fshader_));

  loshader_single_drawer_initialize(&drawer->super, prog, uniblock, 0);
}

void loshader_menu_background_drawer_draw(
    const loshader_menu_background_drawer_t* drawer) {
  assert(drawer != NULL);
  assert(MATH_FLOAT_VALID(drawer->alpha));

  if (drawer->alpha == 0) return;

  glUseProgram(drawer->super.prog);
  glUniform1f(UNIFORM_ALPHA_, drawer->alpha);

  loshader_single_drawer_draw_without_use_program(
      &drawer->super, PRIMITIVE_COUNT_);
}
