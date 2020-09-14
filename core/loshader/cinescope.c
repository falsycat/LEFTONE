#include "./cinescope.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/program.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "./uniblock.h"

/* resources */
#include "anysrc/header.shader.h"
#include "anysrc/cinescope.vshader.h"
#include "anysrc/cinescope.fshader.h"

struct loshader_cinescope_drawer_t {
  const loshader_cinescope_program_t* prog;
  const loshader_uniblock_t*          uniblock;

  gleasy_buffer_uniform_t param;
};

#pragma pack(push, 1)
typedef struct {
  vec4_t color;
  float  size;
} loshader_cinescope_drawer_internal_param_t;
_Static_assert(
    sizeof(float)*5 ==
    sizeof(loshader_cinescope_drawer_internal_param_t));
#pragma pack(pop)

#define LOSHADER_CINESCOPE_UNIBLOCK_INDEX 0
#define LOSHADER_CINESCOPE_PARAM_INDEX    1

#define LOSHADER_CINESCOPE_PRIMITIVE_COUNT 12

void loshader_cinescope_program_initialize(loshader_cinescope_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,    sizeof(loshader_header_shader_),
      loshader_cinescope_vshader_, sizeof(loshader_cinescope_vshader_),
      loshader_cinescope_fshader_, sizeof(loshader_cinescope_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, uniblock, LOSHADER_CINESCOPE_UNIBLOCK_INDEX);

  const GLuint param = glGetUniformBlockIndex(*prog, "param");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, param, LOSHADER_CINESCOPE_PARAM_INDEX);
}

void loshader_cinescope_program_deinitialize(loshader_cinescope_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_cinescope_drawer_t* loshader_cinescope_drawer_new(
    const loshader_cinescope_program_t* prog,
    const loshader_uniblock_t*          uniblock) {
  assert(prog     != NULL);
  assert(uniblock != NULL);

  loshader_cinescope_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };

  glGenBuffers(1, &drawer->param);
  glBindBuffer(GL_UNIFORM_BUFFER, drawer->param);
  glBufferData(GL_UNIFORM_BUFFER,
      sizeof(loshader_cinescope_drawer_internal_param_t), NULL, GL_DYNAMIC_DRAW);

  return drawer;
}

void loshader_cinescope_drawer_delete(loshader_cinescope_drawer_t* drawer) {
  if (drawer == NULL) return;

  glDeleteBuffers(1, &drawer->param);

  memory_delete(drawer);
}

void loshader_cinescope_drawer_set_param(
    loshader_cinescope_drawer_t*             drawer,
    const loshader_cinescope_drawer_param_t* param) {
  assert(drawer != NULL);
  assert(param  != NULL);

  const loshader_cinescope_drawer_internal_param_t p = {
    .size  = param->size,
    .color = param->color,
  };
  glBindBuffer(GL_UNIFORM_BUFFER, drawer->param);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(p), &p);
}

void loshader_cinescope_drawer_draw(const loshader_cinescope_drawer_t* drawer) {
  assert(drawer != NULL);

  glUseProgram(*drawer->prog);

  loshader_uniblock_bind(drawer->uniblock, LOSHADER_CINESCOPE_UNIBLOCK_INDEX);
  glBindBufferBase(GL_UNIFORM_BUFFER,
      LOSHADER_CINESCOPE_PARAM_INDEX, drawer->param);

  glDrawArrays(GL_TRIANGLES, 0, LOSHADER_CINESCOPE_PRIMITIVE_COUNT);
}
