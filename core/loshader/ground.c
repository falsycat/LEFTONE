#include "./ground.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "util/math/algorithm.h"
#include "util/math/matrix.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"
#include "util/gleasy/buffer.h"
#include "util/gleasy/program.h"

#include "./uniblock.h"

/* resources */
#include "anysrc/header.shader.h"
#include "anysrc/ground.vshader.h"
#include "anysrc/ground.fshader.h"

#define LOSHADER_GROUND_VSHADER_IN_GROUND_ID 0
#define LOSHADER_GROUND_VSHADER_IN_POS       1
#define LOSHADER_GROUND_VSHADER_IN_SIZE      2

struct loshader_ground_drawer_t {
  const loshader_ground_program_t* prog;

  const loshader_uniblock_t* uniblock;

  GLuint vao;

  gleasy_buffer_array_t instances;
  size_t                instances_reserved;
  size_t                instances_length;
};

#pragma pack(push, 1)
typedef struct {
  uint16_t ground_id;

  vec2_t pos;
  vec2_t size;
} loshader_ground_drawer_internal_instance_t;
#pragma pack(pop)

#define LOSHADER_GROUND_UNIBLOCK_INDEX 0

#define LOSHADER_GROUND_PRIMITIVE_COUNT 6

static void loshader_ground_program_setup_vao_(
    gleasy_buffer_array_t instances) {
  assert(instances != 0);

  glBindBuffer(GL_ARRAY_BUFFER, instances);

# define enable_attrib_(NAME, name, dim, type) do {  \
    glEnableVertexAttribArray(LOSHADER_GROUND_VSHADER_IN_##NAME);  \
    glVertexAttribPointer(  \
        LOSHADER_GROUND_VSHADER_IN_##NAME,  \
        dim, type, GL_FALSE, sizeof(loshader_ground_drawer_internal_instance_t),  \
        NULL + offsetof(loshader_ground_drawer_internal_instance_t, name));  \
    glVertexAttribDivisor(LOSHADER_GROUND_VSHADER_IN_##NAME, 1);  \
  } while (0)
  enable_attrib_(GROUND_ID, ground_id, 1, GL_UNSIGNED_SHORT);
  enable_attrib_(POS,       pos,       2, GL_FLOAT);
  enable_attrib_(SIZE,      size,      2, GL_FLOAT);
# undef enable_attrib_
}

void loshader_ground_program_initialize(loshader_ground_program_t* prog) {
  *prog = gleasy_program_new(
      loshader_header_shader_,  sizeof(loshader_header_shader_),
      loshader_ground_vshader_, sizeof(loshader_ground_vshader_),
      loshader_ground_fshader_, sizeof(loshader_ground_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);

  glUniformBlockBinding(*prog, uniblock, LOSHADER_GROUND_UNIBLOCK_INDEX);
}
void loshader_ground_program_deinitialize(loshader_ground_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_ground_drawer_t* loshader_ground_drawer_new(
    const loshader_ground_program_t* prog,
    const loshader_uniblock_t*       uniblock) {
  assert(prog     != NULL);
  assert(uniblock != NULL);

  loshader_ground_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };

  glCreateVertexArrays(1, &drawer->vao);
  glBindVertexArray(drawer->vao);

  glGenBuffers(1, &drawer->instances);
  loshader_ground_program_setup_vao_(drawer->instances);
  return drawer;
}

void loshader_ground_drawer_delete(loshader_ground_drawer_t* drawer) {
  if (drawer == NULL) return;

  glDeleteBuffers(1, &drawer->instances);

  glDeleteVertexArrays(1, &drawer->vao);

  memory_delete(drawer);
}

void loshader_ground_drawer_clear(
    loshader_ground_drawer_t* drawer, size_t reserve) {
  assert(drawer != NULL);

  drawer->instances_length = 0;

  if (drawer->instances_reserved < reserve) {
    glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
    glBufferData(GL_ARRAY_BUFFER,
        reserve * sizeof(loshader_ground_drawer_instance_t),
        NULL, GL_DYNAMIC_DRAW);
    drawer->instances_reserved = reserve;
  }
}

void loshader_ground_drawer_add_instance(
    loshader_ground_drawer_t*                drawer,
    const loshader_ground_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  if (drawer->instances_length >= drawer->instances_reserved) {
    fprintf(stderr, "ground drawer instance overflow\n");
    abort();
  }

  const loshader_ground_drawer_internal_instance_t inst = {
    .ground_id = instance->ground_id,
    .pos       = instance->pos,
    .size      = instance->size,
  };

  glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
  glBufferSubData(GL_ARRAY_BUFFER,
      drawer->instances_length*sizeof(inst), sizeof(inst), &inst);

  ++drawer->instances_length;
}

void loshader_ground_drawer_draw(const loshader_ground_drawer_t* drawer) {
  assert(drawer != NULL);

  if (drawer->instances_length == 0) return;

  glUseProgram(*drawer->prog);
  glBindVertexArray(drawer->vao);

  loshader_uniblock_bind(
      drawer->uniblock, LOSHADER_GROUND_UNIBLOCK_INDEX);

  glDrawArraysInstanced(GL_TRIANGLES,
      0, LOSHADER_GROUND_PRIMITIVE_COUNT, drawer->instances_length);
}

