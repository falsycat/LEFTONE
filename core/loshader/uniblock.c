#include "./uniblock.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/math/matrix.h"

#include "core/locommon/position.h"
#include "core/locommon/screen.h"

#define AA_ .00001f

#pragma pack(push, 1)
typedef struct {
  vec2_t resolution;
  vec2_t dpi;
  float  aa;

  int8_t padding[12];

  mat4_t proj;
  mat4_t camera;
  vec4_t pos;
  float  time;
} loshader_uniblock_internal_t;
_Static_assert(
    sizeof(float)*5 + 12 + sizeof(float)*37 ==
    sizeof(loshader_uniblock_internal_t),
    "recheck the type has no padding");
#pragma pack(pop)

bool loshader_uniblock_param_valid(const loshader_uniblock_param_t* param) {
  return
      param != NULL &&
      mat4_valid(&param->proj) &&
      mat4_valid(&param->cam) &&
      locommon_position_valid(&param->pos) &&
      MATH_FLOAT_VALID(param->time);
}

void loshader_uniblock_initialize(
    loshader_uniblock_t* uni, const locommon_screen_t* screen) {
  assert(uni != NULL);
  assert(locommon_screen_valid(screen));

  *uni = (typeof(*uni)) {
    .screen = screen,
  };

  glGenBuffers(1, &uni->buf);
  glBindBuffer(GL_UNIFORM_BUFFER, uni->buf);
  glBufferData(GL_UNIFORM_BUFFER,
      sizeof(loshader_uniblock_internal_t), NULL, GL_DYNAMIC_DRAW);
}

void loshader_uniblock_deinitialize(loshader_uniblock_t* uni) {
  assert(uni != NULL);

  glDeleteBuffers(1, &uni->buf);
}

void loshader_uniblock_update_param(
    loshader_uniblock_t*             uni,
    const loshader_uniblock_param_t* param) {
  assert(uni != NULL);
  assert(loshader_uniblock_param_valid(param));

  const vec4_t pos = vec4(
      param->pos.chunk.x,
      param->pos.chunk.y,
      param->pos.fract.x,
      param->pos.fract.y);

  const loshader_uniblock_internal_t internal = {
    .resolution = uni->screen->resolution,
    .dpi        = uni->screen->dpi,
    .aa         = MATH_MAX(uni->screen->dpi.x, uni->screen->dpi.y) * AA_,
    .proj       = param->proj,
    .camera     = param->cam,
    .pos        = pos,
    .time       = param->time,
  };
  glBindBuffer(GL_UNIFORM_BUFFER, uni->buf);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(internal), &internal);
}

void loshader_uniblock_bind(const loshader_uniblock_t* uni, GLuint index) {
  assert(uni != NULL);

  glBindBufferBase(GL_UNIFORM_BUFFER, index, uni->buf);
}
