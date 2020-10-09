#include "./hud_text.h"

#include <assert.h>

#include "util/gleasy/program.h"
#include "util/gleasy/texture.h"
#include "util/glyphas/drawer.h"

#include "./text.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/hud_text.vshader.h"
#include "core/loshader/anysrc/hud_text.fshader.h"

#define UNIFORM_ALPHA_ 1  /* 0 is used by the super class */

void loshader_hud_text_drawer_initialize(
    loshader_hud_text_drawer_t* drawer,
    const loshader_uniblock_t*  uniblock,
    gleasy_texture_2d_t         tex) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);
  assert(tex != 0);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,    sizeof(loshader_header_shader_),
      loshader_hud_text_vshader_, sizeof(loshader_hud_text_vshader_),
      loshader_hud_text_fshader_, sizeof(loshader_hud_text_fshader_));

  loshader_text_drawer_initialize(&drawer->super, prog, uniblock, tex);
}

void loshader_hud_text_drawer_draw(const loshader_hud_text_drawer_t* drawer) {
  assert(drawer != NULL);

  glUseProgram(drawer->super.prog);
  glUniform1f(UNIFORM_ALPHA_, drawer->alpha);

  loshader_text_drawer_draw_without_use_program(&drawer->super);
}
