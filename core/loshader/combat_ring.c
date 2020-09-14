#include "./combat_ring.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/program.h"
#include "util/math/algorithm.h"
#include "util/math/constant.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "./uniblock.h"

/* resources */
#include "anysrc/header.shader.h"
#include "anysrc/combat_ring.vshader.h"
#include "anysrc/combat_ring.fshader.h"

#define LOSHADER_COMBAT_RING_VSHADER_IN_RANGE  0
#define LOSHADER_COMBAT_RING_VSHADER_IN_PERIOD 1
#define LOSHADER_COMBAT_RING_VSHADER_IN_COLOR  2

struct loshader_combat_ring_drawer_t {
  const loshader_combat_ring_program_t* prog;
  const loshader_uniblock_t*            uniblock;

  GLuint vao;

  gleasy_buffer_array_t instances;
  size_t                instances_reserved;
  size_t                instances_length;
};

#pragma pack(push, 1)
typedef struct {
  float  range;
  vec2_t period;  /* x~y */
  vec4_t color;
} loshader_combat_ring_drawer_internal_instance_t;
#pragma pack(pop)

#define LOSHADER_COMBAT_RING_UNIBLOCK_INDEX 0

#define LOSHADER_COMBAT_RING_PRIMITIVE_COUNT 6

static void loshader_combat_ring_program_setup_vao_(
    gleasy_buffer_array_t instances) {
  assert(instances != 0);

  glBindBuffer(GL_ARRAY_BUFFER, instances);

  glEnableVertexAttribArray(LOSHADER_COMBAT_RING_VSHADER_IN_RANGE);
  glVertexAttribPointer(
      LOSHADER_COMBAT_RING_VSHADER_IN_RANGE, 1, GL_FLOAT, GL_FALSE,
      sizeof(loshader_combat_ring_drawer_internal_instance_t),
      NULL + offsetof(loshader_combat_ring_drawer_internal_instance_t, range));
  glVertexAttribDivisor(LOSHADER_COMBAT_RING_VSHADER_IN_RANGE, 1);

  glEnableVertexAttribArray(LOSHADER_COMBAT_RING_VSHADER_IN_PERIOD);
  glVertexAttribPointer(
      LOSHADER_COMBAT_RING_VSHADER_IN_PERIOD, 2, GL_FLOAT, GL_FALSE,
      sizeof(loshader_combat_ring_drawer_internal_instance_t),
      NULL + offsetof(loshader_combat_ring_drawer_internal_instance_t, period));
  glVertexAttribDivisor(LOSHADER_COMBAT_RING_VSHADER_IN_PERIOD, 1);

  glEnableVertexAttribArray(LOSHADER_COMBAT_RING_VSHADER_IN_COLOR);
  glVertexAttribPointer(
      LOSHADER_COMBAT_RING_VSHADER_IN_COLOR, 4, GL_FLOAT, GL_FALSE,
      sizeof(loshader_combat_ring_drawer_internal_instance_t),
      NULL + offsetof(loshader_combat_ring_drawer_internal_instance_t, color));
  glVertexAttribDivisor(LOSHADER_COMBAT_RING_VSHADER_IN_COLOR, 1);
}

void loshader_combat_ring_program_initialize(
    loshader_combat_ring_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,       sizeof(loshader_header_shader_),
      loshader_combat_ring_vshader_, sizeof(loshader_combat_ring_vshader_),
      loshader_combat_ring_fshader_, sizeof(loshader_combat_ring_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);

  glUniformBlockBinding(*prog, uniblock, LOSHADER_COMBAT_RING_UNIBLOCK_INDEX);
}

void loshader_combat_ring_program_deinitialize(
    loshader_combat_ring_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_combat_ring_drawer_t* loshader_combat_ring_drawer_new(
    const loshader_combat_ring_program_t* prog,
    const loshader_uniblock_t*            uniblock) {
  assert(prog     != NULL);
  assert(uniblock != NULL);

  loshader_combat_ring_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };

  glCreateVertexArrays(1, &drawer->vao);
  glBindVertexArray(drawer->vao);

  glGenBuffers(1, &drawer->instances);
  loshader_combat_ring_program_setup_vao_(drawer->instances);

  return drawer;
}

void loshader_combat_ring_drawer_delete(loshader_combat_ring_drawer_t* drawer) {
  if (drawer == NULL) return;

  glDeleteBuffers(1, &drawer->instances);
  glDeleteVertexArrays(1, &drawer->vao);
  memory_delete(drawer);
}

void loshader_combat_ring_drawer_clear(
    loshader_combat_ring_drawer_t* drawer, size_t reserve) {
  assert(drawer != NULL);

  drawer->instances_length = 0;

  if (drawer->instances_reserved < reserve) {
    glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
    glBufferData(GL_ARRAY_BUFFER,
        reserve * sizeof(loshader_combat_ring_drawer_internal_instance_t),
        NULL, GL_DYNAMIC_DRAW);
    drawer->instances_reserved = reserve;
  }
}

void loshader_combat_ring_drawer_add_instance(
    loshader_combat_ring_drawer_t*                drawer,
    const loshader_combat_ring_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  if (drawer->instances_length >= drawer->instances_reserved) {
    fprintf(stderr, "combat ring drawer instance overflow\n");
    abort();
  }

  const size_t offset = drawer->instances_length *
      sizeof(loshader_combat_ring_drawer_internal_instance_t);

  const loshader_combat_ring_drawer_internal_instance_t i = {
    .range  = instance->range,
    .period = vec2(instance->start, instance->end),
    .color  = instance->color,
  };
  glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
  glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(i), &i);

  ++drawer->instances_length;
}

void loshader_combat_ring_drawer_draw(
    const loshader_combat_ring_drawer_t* drawer) {
  assert(drawer != NULL);

  if (drawer->instances_length == 0) return;

  glUseProgram(*drawer->prog);
  glBindVertexArray(drawer->vao);

  loshader_uniblock_bind(drawer->uniblock, LOSHADER_COMBAT_RING_UNIBLOCK_INDEX);

  glDrawArraysInstanced(GL_TRIANGLES,
      0, LOSHADER_COMBAT_RING_PRIMITIVE_COUNT, drawer->instances_length);
}
