#include "./fog.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/program.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "core/locommon/position.h"

#include "./uniblock.h"

/* resources */
#include "anysrc/header.shader.h"
#include "anysrc/fog.vshader.h"
#include "anysrc/fog.fshader.h"

struct loshader_fog_drawer_t {
  const loshader_fog_program_t* prog;
  const loshader_uniblock_t*         uniblock;

  gleasy_buffer_uniform_t param;
};

#pragma pack(push, 1)
typedef struct {
  float type;
  float prev_type;
  float transition;

  float  bounds_fog;
  vec4_t bounds_pos;
  vec2_t bounds_size;
} loshader_fog_drawer_internal_param_t;
_Static_assert(
    sizeof(float)*10 ==
    sizeof(loshader_fog_drawer_internal_param_t));
#pragma pack(pop)

#define LOSHADER_FOG_UNIBLOCK_INDEX 0
#define LOSHADER_FOG_PARAM_INDEX    1

#define LOSHADER_FOG_PRIMITIVE_COUNT 6

void loshader_fog_program_initialize(loshader_fog_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,    sizeof(loshader_header_shader_),
      loshader_fog_vshader_, sizeof(loshader_fog_vshader_),
      loshader_fog_fshader_, sizeof(loshader_fog_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, uniblock, LOSHADER_FOG_UNIBLOCK_INDEX);

  const GLuint param = glGetUniformBlockIndex(*prog, "param");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, param, LOSHADER_FOG_PARAM_INDEX);
}

void loshader_fog_program_deinitialize(loshader_fog_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_fog_drawer_t* loshader_fog_drawer_new(
    const loshader_fog_program_t* prog,
    const loshader_uniblock_t*    uniblock) {
  assert(prog     != NULL);
  assert(uniblock != NULL);

  loshader_fog_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };

  glGenBuffers(1, &drawer->param);
  glBindBuffer(GL_UNIFORM_BUFFER, drawer->param);
  glBufferData(GL_UNIFORM_BUFFER,
      sizeof(loshader_fog_drawer_internal_param_t), NULL, GL_DYNAMIC_DRAW);

  return drawer;
}

void loshader_fog_drawer_delete(loshader_fog_drawer_t* drawer) {
  if (drawer == NULL) return;

  glDeleteBuffers(1, &drawer->param);

  memory_delete(drawer);
}

void loshader_fog_drawer_set_param(
    loshader_fog_drawer_t*             drawer,
    const loshader_fog_drawer_param_t* param) {
  assert(drawer != NULL);
  assert(param  != NULL);

  const loshader_fog_drawer_internal_param_t p = {
    .type        = param->type,
    .prev_type   = param->prev_type,
    .transition  = param->transition,
    .bounds_fog  = param->bounds_fog,
    .bounds_pos  = vec4(
        param->bounds_pos.chunk.x,
        param->bounds_pos.chunk.y,
        param->bounds_pos.fract.x,
        param->bounds_pos.fract.y),
    .bounds_size = param->bounds_size,
  };
  glBindBuffer(GL_UNIFORM_BUFFER, drawer->param);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(p), &p);
}

void loshader_fog_drawer_draw(const loshader_fog_drawer_t* drawer) {
  assert(drawer != NULL);

  glUseProgram(*drawer->prog);

  loshader_uniblock_bind(drawer->uniblock, LOSHADER_FOG_UNIBLOCK_INDEX);
  glBindBufferBase(GL_UNIFORM_BUFFER,
      LOSHADER_FOG_PARAM_INDEX, drawer->param);

  glDrawArrays(GL_TRIANGLES, 0, LOSHADER_FOG_PRIMITIVE_COUNT);
}
