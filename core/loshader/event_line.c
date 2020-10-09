#include "./event_line.h"

#include <assert.h>

#include "util/gleasy/program.h"
#include "util/gleasy/texture.h"

#include "./text.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/event_line.vshader.h"
#include "core/loshader/anysrc/event_line.fshader.h"

#define UNIBLOCK_INDEX_ 0

void loshader_event_line_drawer_initialize(
    loshader_event_line_drawer_t* drawer,
    const loshader_uniblock_t*    uniblock,
    gleasy_texture_2d_t           tex) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);
  assert(tex != 0);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,      sizeof(loshader_header_shader_),
      loshader_event_line_vshader_, sizeof(loshader_event_line_vshader_),
      loshader_event_line_fshader_, sizeof(loshader_event_line_fshader_));

  loshader_text_drawer_initialize(&drawer->super, prog, uniblock, tex);
}
