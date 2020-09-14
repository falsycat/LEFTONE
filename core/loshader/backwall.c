#include "./backwall.h"

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
#include "anysrc/backwall.vshader.h"
#include "anysrc/backwall.fshader.h"

struct loshader_backwall_drawer_t {
  const loshader_backwall_program_t* prog;
  const loshader_uniblock_t*         uniblock;

  gleasy_buffer_uniform_t param;
};

#pragma pack(push, 1)
typedef struct {
  float type;
  float prev_type;
  float transition;
} loshader_backwall_drawer_internal_param_t;
_Static_assert(
    sizeof(float)*3 ==
    sizeof(loshader_backwall_drawer_internal_param_t));
#pragma pack(pop)

#define LOSHADER_BACKWALL_UNIBLOCK_INDEX 0
#define LOSHADER_BACKWALL_PARAM_INDEX    1

#define LOSHADER_BACKWALL_PRIMITIVE_COUNT 6

void loshader_backwall_program_initialize(loshader_backwall_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,    sizeof(loshader_header_shader_),
      loshader_backwall_vshader_, sizeof(loshader_backwall_vshader_),
      loshader_backwall_fshader_, sizeof(loshader_backwall_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, uniblock, LOSHADER_BACKWALL_UNIBLOCK_INDEX);

  const GLuint param = glGetUniformBlockIndex(*prog, "param");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, param, LOSHADER_BACKWALL_PARAM_INDEX);
}

void loshader_backwall_program_deinitialize(loshader_backwall_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_backwall_drawer_t* loshader_backwall_drawer_new(
    const loshader_backwall_program_t* prog,
    const loshader_uniblock_t*         uniblock) {
  assert(prog     != NULL);
  assert(uniblock != NULL);

  loshader_backwall_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };

  glGenBuffers(1, &drawer->param);
  glBindBuffer(GL_UNIFORM_BUFFER, drawer->param);
  glBufferData(GL_UNIFORM_BUFFER,
      sizeof(loshader_backwall_drawer_internal_param_t), NULL, GL_DYNAMIC_DRAW);

  return drawer;
}

void loshader_backwall_drawer_delete(loshader_backwall_drawer_t* drawer) {
  if (drawer == NULL) return;

  glDeleteBuffers(1, &drawer->param);

  memory_delete(drawer);
}

void loshader_backwall_drawer_set_param(
    loshader_backwall_drawer_t*             drawer,
    const loshader_backwall_drawer_param_t* param) {
  assert(drawer != NULL);
  assert(param  != NULL);

  const loshader_backwall_drawer_internal_param_t p = {
    .type       = param->type,
    .prev_type  = param->prev_type,
    .transition = param->transition,
  };
  glBindBuffer(GL_UNIFORM_BUFFER, drawer->param);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(p), &p);
}

void loshader_backwall_drawer_draw(const loshader_backwall_drawer_t* drawer) {
  assert(drawer != NULL);

  glUseProgram(*drawer->prog);

  loshader_uniblock_bind(drawer->uniblock, LOSHADER_BACKWALL_UNIBLOCK_INDEX);
  glBindBufferBase(GL_UNIFORM_BUFFER,
      LOSHADER_BACKWALL_PARAM_INDEX, drawer->param);

  glDrawArrays(GL_TRIANGLES, 0, LOSHADER_BACKWALL_PRIMITIVE_COUNT);
}
