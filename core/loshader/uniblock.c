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
#include "util/memory/memory.h"

#include "core/locommon/position.h"

struct loshader_uniblock_t {
  gleasy_buffer_uniform_t buf;
};

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
    sizeof(loshader_uniblock_internal_t));
#pragma pack(pop)

bool loshader_uniblock_param_valid(const loshader_uniblock_param_t* param) {
  return
      param != NULL &&
      mat4_valid(&param->proj) &&
      mat4_valid(&param->cam) &&
      locommon_position_valid(&param->pos) &&
      MATH_FLOAT_VALID(param->time);
}

loshader_uniblock_t* loshader_uniblock_new(void) {
  loshader_uniblock_t* uni = memory_new(sizeof(*uni));
  *uni = (typeof(*uni)) {0};

  glGenBuffers(1, &uni->buf);
  glBindBuffer(GL_UNIFORM_BUFFER, uni->buf);
  glBufferData(GL_UNIFORM_BUFFER,
      sizeof(loshader_uniblock_internal_t), NULL, GL_DYNAMIC_DRAW);

  return uni;
}

void loshader_uniblock_delete(loshader_uniblock_t* uni) {
  if (uni == NULL) return;

  glDeleteBuffers(1, &uni->buf);
  memory_delete(uni);
}

void loshader_uniblock_update_display_param(
    loshader_uniblock_t* uni, const vec2_t* resolution, const vec2_t* dpi) {
  assert(uni != NULL);
  assert(vec2_valid(resolution));
  assert(vec2_valid(dpi));

  const loshader_uniblock_internal_t internal = {
    .resolution = *resolution,
    .dpi        = *dpi,
    .aa         = MATH_MAX(dpi->x, dpi->y) / 100000,
  };

  static const size_t size =
      offsetof(loshader_uniblock_internal_t, aa) + sizeof(internal.aa);

  glBindBuffer(GL_UNIFORM_BUFFER, uni->buf);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, size, &internal);
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
    .proj   = param->proj,
    .camera = param->cam,
    .pos    = pos,
    .time   = param->time,
  };

  static const size_t offset = offsetof(loshader_uniblock_internal_t, proj);

  glBindBuffer(GL_UNIFORM_BUFFER, uni->buf);
  glBufferSubData(GL_UNIFORM_BUFFER,
      offset, sizeof(internal)-offset, &internal.proj);
}

void loshader_uniblock_bind(const loshader_uniblock_t* uni, GLuint index) {
  assert(uni != NULL);

  glBindBufferBase(GL_UNIFORM_BUFFER, index, uni->buf);
}
