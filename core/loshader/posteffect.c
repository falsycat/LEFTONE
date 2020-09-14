#include "./posteffect.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/framebuffer.h"
#include "util/gleasy/program.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "./uniblock.h"

/* resources */
#include "anysrc/header.shader.h"
#include "anysrc/posteffect.vshader.h"
#include "anysrc/posteffect.fshader.h"

#define LOSHADER_POSTEFFECT_UNIFORM_SRC 0

struct loshader_posteffect_drawer_t {
  const loshader_posteffect_program_t* prog;
  const loshader_uniblock_t*           uniblock;

  const gleasy_framebuffer_t* fb;

  gleasy_buffer_uniform_t param;
};

#pragma pack(push, 1)
typedef struct {
  float whole_blur;
  float raster;

  float radial_displacement;
  float amnesia_displacement;
  float radial_fade;

  float brightness;
} loshader_posteffect_drawer_param_internal_t;
_Static_assert(
    sizeof(float)*6 ==
    sizeof(loshader_posteffect_drawer_param_internal_t));
#pragma pack(pop)

#define LOSHADER_POSTEFFECT_UNIBLOCK_INDEX 0
#define LOSHADER_POSTEFFECT_PARAM_INDEX    1

void loshader_posteffect_program_initialize(
    loshader_posteffect_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,      sizeof(loshader_header_shader_),
      loshader_posteffect_vshader_, sizeof(loshader_posteffect_vshader_),
      loshader_posteffect_fshader_, sizeof(loshader_posteffect_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, uniblock, LOSHADER_POSTEFFECT_UNIBLOCK_INDEX);

  const GLuint param = glGetUniformBlockIndex(*prog, "param");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, param, LOSHADER_POSTEFFECT_PARAM_INDEX);
}

void loshader_posteffect_program_deinitialize(
    loshader_posteffect_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_posteffect_drawer_t* loshader_posteffect_drawer_new(
    const loshader_posteffect_program_t* prog,
    const loshader_uniblock_t*           uniblock,
    const gleasy_framebuffer_t*          fb) {
  assert(prog     != NULL);
  assert(uniblock != NULL);
  assert(fb       != NULL);

  loshader_posteffect_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
    .fb       = fb,
  };

  glGenBuffers(1, &drawer->param);
  glBindBuffer(GL_UNIFORM_BUFFER, drawer->param);
  glBufferData(GL_UNIFORM_BUFFER,
      sizeof(loshader_posteffect_drawer_param_internal_t),
      NULL,
      GL_DYNAMIC_DRAW);

  return drawer;
}

void loshader_posteffect_drawer_delete(loshader_posteffect_drawer_t* drawer) {
  if (drawer == NULL) return;

  glDeleteBuffers(1, &drawer->param);

  memory_delete(drawer);
}

void loshader_posteffect_drawer_set_param(
    loshader_posteffect_drawer_t*             drawer,
    const loshader_posteffect_drawer_param_t* param) {
  assert(drawer != NULL);
  assert(param  != NULL);

  const loshader_posteffect_drawer_param_internal_t p = {
    .whole_blur           = param->whole_blur,
    .raster               = param->raster,
    .radial_displacement  = param->radial_displacement,
    .amnesia_displacement = param->amnesia_displacement,
    .radial_fade          = param->radial_fade,
    .brightness           = param->brightness,
  };
  glBindBuffer(GL_UNIFORM_BUFFER, drawer->param);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(p), &p);
}

void loshader_posteffect_drawer_draw(
    const loshader_posteffect_drawer_t* drawer) {
  assert(drawer != NULL);

  glUseProgram(*drawer->prog);

  loshader_uniblock_bind(drawer->uniblock, LOSHADER_POSTEFFECT_UNIBLOCK_INDEX);
  glBindBufferBase(GL_UNIFORM_BUFFER,
      LOSHADER_POSTEFFECT_PARAM_INDEX, drawer->param);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, drawer->fb->colorbuf);
  glUniform1i(LOSHADER_POSTEFFECT_UNIFORM_SRC, 0);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}
