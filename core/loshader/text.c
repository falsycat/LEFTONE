#include "./text.h"

#include <assert.h>

#include "util/gleasy/program.h"
#include "util/gleasy/texture.h"
#include "util/glyphas/drawer.h"

#include "./uniblock.h"

#define UNIBLOCK_INDEX_ 0

void loshader_text_drawer_initialize(
    loshader_text_drawer_t*    drawer,
    gleasy_program_t           prog,
    const loshader_uniblock_t* uniblock,
    gleasy_texture_2d_t        tex) {
  assert(drawer   != NULL);
  assert(prog != 0);
  assert(uniblock != NULL);
  assert(tex != 0);

  const GLuint uniblock_index = glGetUniformBlockIndex(prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(prog, uniblock_index, UNIBLOCK_INDEX_);

  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };
  glyphas_drawer_initialize(&drawer->super, tex);
}

void loshader_text_drawer_deinitialize(
    loshader_text_drawer_t* drawer) {
  assert(drawer != NULL);

  glyphas_drawer_deinitialize(&drawer->super);
  glDeleteProgram(drawer->prog);
}

void loshader_text_drawer_draw(const loshader_text_drawer_t* drawer) {
  assert(drawer != NULL);

  glUseProgram(drawer->prog);
  loshader_text_drawer_draw_without_use_program(drawer);
}

void loshader_text_drawer_draw_without_use_program(
    const loshader_text_drawer_t* drawer) {
  assert(drawer != NULL);

  loshader_uniblock_bind(drawer->uniblock, UNIBLOCK_INDEX_);
  glyphas_drawer_draw(&drawer->super);
}
