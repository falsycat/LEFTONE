#include "./bullet.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/program.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "./uniblock.h"

/* resources */
#include "anysrc/header.shader.h"
#include "anysrc/bullet.vshader.h"
#include "anysrc/bullet.fshader.h"

#define LOSHADER_BULLET_VSHADER_IN_BULLET_ID 0
#define LOSHADER_BULLET_VSHADER_IN_POS       1
#define LOSHADER_BULLET_VSHADER_IN_SIZE      2
#define LOSHADER_BULLET_VSHADER_IN_THETA     3
#define LOSHADER_BULLET_VSHADER_IN_TIME      4
#define LOSHADER_BULLET_VSHADER_IN_COLOR     5

struct loshader_bullet_drawer_t {
  const loshader_bullet_program_t* prog;
  const loshader_uniblock_t*       uniblock;

  GLuint vao;

  gleasy_buffer_array_t instances;
  size_t                instances_reserved;
  size_t                instances_length;
};

#pragma pack(push, 1)
typedef struct {
  uint16_t bullet_id;

  vec2_t pos;
  vec2_t size;
  float  theta;
  float  time;

  vec4_t color;
} loshader_bullet_drawer_internal_instance_t;
#pragma pack(pop)

#define LOSHADER_BULLET_UNIBLOCK_INDEX 0

#define LOSHADER_BULLET_PRIMITIVE_COUNT 6

static void loshader_bullet_program_setup_vao_(
    gleasy_buffer_array_t instances) {
  assert(instances != 0);

  glBindBuffer(GL_ARRAY_BUFFER, instances);

# define enable_attrib_(NAME, name, dim, type) do {  \
    glEnableVertexAttribArray(LOSHADER_BULLET_VSHADER_IN_##NAME);  \
    glVertexAttribPointer(  \
        LOSHADER_BULLET_VSHADER_IN_##NAME, dim, type, GL_FALSE,  \
        sizeof(loshader_bullet_drawer_internal_instance_t),  \
        NULL + offsetof(loshader_bullet_drawer_internal_instance_t, name));  \
    glVertexAttribDivisor(LOSHADER_BULLET_VSHADER_IN_##NAME, 1);  \
  } while (0)

  enable_attrib_(BULLET_ID, bullet_id, 1, GL_UNSIGNED_SHORT);

  enable_attrib_(POS,   pos,   2, GL_FLOAT);
  enable_attrib_(SIZE,  size,  2, GL_FLOAT);
  enable_attrib_(THETA, theta, 1, GL_FLOAT);
  enable_attrib_(TIME,  time,  1, GL_FLOAT);

  enable_attrib_(COLOR, color, 4, GL_FLOAT);

# undef enable_attrib_
}

void loshader_bullet_program_initialize(loshader_bullet_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,  sizeof(loshader_header_shader_),
      loshader_bullet_vshader_, sizeof(loshader_bullet_vshader_),
      loshader_bullet_fshader_, sizeof(loshader_bullet_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);

  glUniformBlockBinding(*prog, uniblock, LOSHADER_BULLET_UNIBLOCK_INDEX);
}

void loshader_bullet_program_deinitialize(loshader_bullet_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_bullet_drawer_t* loshader_bullet_drawer_new(
    const loshader_bullet_program_t* prog,
    const loshader_uniblock_t*       uniblock) {
  assert(prog     != NULL);
  assert(uniblock != NULL);

  loshader_bullet_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };

  glCreateVertexArrays(1, &drawer->vao);
  glBindVertexArray(drawer->vao);

  glGenBuffers(1, &drawer->instances);
  loshader_bullet_program_setup_vao_(drawer->instances);

  return drawer;
}

void loshader_bullet_drawer_delete(loshader_bullet_drawer_t* drawer) {
  assert(drawer != NULL);

  glDeleteBuffers(1, &drawer->instances);

  glDeleteVertexArrays(1, &drawer->vao);

  memory_delete(drawer);
}

void loshader_bullet_drawer_clear(
    loshader_bullet_drawer_t* drawer, size_t reserve) {
  assert(drawer  != NULL);
  assert(reserve > 0);

  drawer->instances_length = 0;

  if (drawer->instances_reserved < reserve) {
    glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
    glBufferData(GL_ARRAY_BUFFER,
        reserve * sizeof(loshader_bullet_drawer_internal_instance_t),
        NULL, GL_DYNAMIC_DRAW);
    drawer->instances_reserved = reserve;
  }
}

void loshader_bullet_drawer_add_instance(
    loshader_bullet_drawer_t*                drawer,
    const loshader_bullet_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  if (drawer->instances_length >= drawer->instances_reserved) {
    fprintf(stderr, "bullet drawer instance overflow\n");
    abort();
  }

  const loshader_bullet_drawer_internal_instance_t insta = {
    .bullet_id = instance->bullet_id,
    .pos       = instance->pos,
    .size      = instance->size,
    .theta     = instance->theta,
    .time      = instance->time,
    .color     = instance->color,
  };

  const size_t offset = drawer->instances_length * sizeof(insta);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
  glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(insta), &insta);

  ++drawer->instances_length;
}

void loshader_bullet_drawer_draw(const loshader_bullet_drawer_t* drawer) {
  assert(drawer != NULL);

  if (drawer->instances_length == 0) return;

  glUseProgram(*drawer->prog);
  glBindVertexArray(drawer->vao);

  loshader_uniblock_bind(drawer->uniblock, LOSHADER_BULLET_UNIBLOCK_INDEX);

  glDrawArraysInstanced(GL_TRIANGLES,
      0, LOSHADER_BULLET_PRIMITIVE_COUNT, drawer->instances_length);
}
