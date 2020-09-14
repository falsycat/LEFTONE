#include "./menu_text.h"

#include <assert.h>
#include <stddef.h>

#include "util/gleasy/program.h"
#include "util/gleasy/texture.h"
#include "util/glyphas/block.h"
#include "util/glyphas/drawer.h"

#include "./uniblock.h"

/* resources */
#include "anysrc/header.shader.h"
#include "anysrc/menu_text.vshader.h"
#include "anysrc/menu_text.fshader.h"

#define LOSHADER_MENU_TEXT_UNIFORM_ALPHA 1

#define LOSHADER_MENU_TEXT_UNIBLOCK_INDEX 0

void loshader_menu_text_program_initialize(
    loshader_menu_text_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,    sizeof(loshader_header_shader_),
      loshader_menu_text_vshader_, sizeof(loshader_menu_text_vshader_),
      loshader_menu_text_fshader_, sizeof(loshader_menu_text_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, uniblock, LOSHADER_MENU_TEXT_UNIBLOCK_INDEX);
}

void loshader_menu_text_program_deinitialize(
    loshader_menu_text_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

void loshader_menu_text_drawer_initialize(
    loshader_menu_text_drawer_t*        drawer,
    const loshader_menu_text_program_t* prog,
    const loshader_uniblock_t*         uniblock,
    gleasy_texture_2d_t                tex) {
  assert(drawer   != NULL);
  assert(prog     != NULL);
  assert(uniblock != NULL);
  assert(tex != 0);

  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
    .tex      = tex,

    .glyphas = glyphas_drawer_new(),

    .alpha = 1,
  };
}

void loshader_menu_text_drawer_deinitialize(
    loshader_menu_text_drawer_t* drawer) {
  assert(drawer != NULL);

  glyphas_drawer_delete(drawer->glyphas);
}

void loshader_menu_text_drawer_clear(
    loshader_menu_text_drawer_t* drawer, size_t reserve) {
  assert(drawer != NULL);
  assert(reserve > 0);

  glyphas_drawer_clear(drawer->glyphas, drawer->tex, reserve);
}

void loshader_menu_text_drawer_add_block(
    loshader_menu_text_drawer_t* drawer, const glyphas_block_t* block) {
  assert(drawer != NULL);
  assert(block  != NULL);

  glyphas_drawer_add_block(drawer->glyphas, block);
}

void loshader_menu_text_drawer_draw(const loshader_menu_text_drawer_t* drawer) {
  assert(drawer != NULL);

  glUseProgram(*drawer->prog);

  loshader_uniblock_bind(drawer->uniblock, LOSHADER_MENU_TEXT_UNIBLOCK_INDEX);

  glUniform1f(LOSHADER_MENU_TEXT_UNIFORM_ALPHA, drawer->alpha);

  glyphas_drawer_draw(drawer->glyphas);
}
