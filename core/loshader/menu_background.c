#include "./menu_background.h"

#include <assert.h>

#include <GL/glew.h>

#include "util/gleasy/program.h"
#include "util/math/algorithm.h"
#include "util/memory/memory.h"

#include "./uniblock.h"

/* resources */
#include "anysrc/header.shader.h"
#include "anysrc/menu_background.vshader.h"
#include "anysrc/menu_background.fshader.h"

#define LOSHADER_MENU_BACKGROUND_UNIFORM_ALPHA 0

struct loshader_menu_background_drawer_t {
  const loshader_menu_background_program_t* prog;
  const loshader_uniblock_t*                uniblock;

  float alpha;
};

#define LOSHADER_MENU_BACKGROUND_UNIBLOCK_INDEX 0

void loshader_menu_background_program_initialize(
    loshader_menu_background_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,
      sizeof(loshader_header_shader_),
      loshader_menu_background_vshader_,
      sizeof(loshader_menu_background_vshader_),
      loshader_menu_background_fshader_,
      sizeof(loshader_menu_background_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(
      *prog, uniblock, LOSHADER_MENU_BACKGROUND_UNIBLOCK_INDEX);
}

void loshader_menu_background_program_deinitialize(
    loshader_menu_background_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_menu_background_drawer_t* loshader_menu_background_drawer_new(
    const loshader_menu_background_program_t* prog,
    const loshader_uniblock_t*                uniblock) {
  assert(prog     != NULL);
  assert(uniblock != NULL);

  loshader_menu_background_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };
  return drawer;
}

void loshader_menu_background_drawer_delete(
    loshader_menu_background_drawer_t* drawer) {
  if (drawer == NULL) return;

  memory_delete(drawer);
}

void loshader_menu_background_drawer_set_alpha(
    loshader_menu_background_drawer_t* drawer, float alpha) {
  assert(drawer != NULL);
  assert(MATH_FLOAT_VALID(alpha));

  drawer->alpha = alpha;
}

void loshader_menu_background_drawer_draw(
    const loshader_menu_background_drawer_t* drawer) {
  assert(drawer != NULL);

  if (drawer->alpha == 0) return;

  glUseProgram(*drawer->prog);

  loshader_uniblock_bind(
      drawer->uniblock, LOSHADER_MENU_BACKGROUND_UNIBLOCK_INDEX);

  glUniform1f(LOSHADER_MENU_BACKGROUND_UNIFORM_ALPHA, drawer->alpha);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}
