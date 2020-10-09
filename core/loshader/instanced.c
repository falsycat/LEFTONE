#include "./instanced.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/program.h"

#include "./uniblock.h"

#define UNIBLOCK_INDEX_ 0

void loshader_instanced_drawer_initialize(
    loshader_instanced_drawer_t* drawer,
    gleasy_program_t             prog,
    const loshader_uniblock_t*   uniblock,
    size_t                       instance_size) {
  assert(drawer != NULL);
  assert(uniblock != NULL);
  assert(instance_size > 0);

  const GLuint uniblock_index = glGetUniformBlockIndex(prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(prog, uniblock_index, UNIBLOCK_INDEX_);

  *drawer = (typeof(*drawer)) {
    .prog          = prog,
    .uniblock      = uniblock,
    .instance_size = instance_size,
  };

  glCreateVertexArrays(1, &drawer->vao);
  glGenBuffers(1, &drawer->instances);
}

void loshader_instanced_drawer_deinitialize(
    loshader_instanced_drawer_t* drawer) {
  assert(drawer != NULL);

  glDeleteVertexArrays(1, &drawer->vao);
  glDeleteBuffers(1, &drawer->instances);
  glDeleteProgram(drawer->prog);
}

void loshader_instanced_drawer_clear(
    loshader_instanced_drawer_t* drawer, size_t reserve) {
  assert(drawer != NULL);
  assert(reserve > 0);

  drawer->instances_length = 0;

  if (drawer->instances_reserved < reserve) {
    glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
    glBufferData(GL_ARRAY_BUFFER,
        reserve*drawer->instance_size, NULL, GL_DYNAMIC_DRAW);
    drawer->instances_reserved = reserve;
  }
}

bool loshader_instanced_drawer_add_instance(
    loshader_instanced_drawer_t* drawer, const void* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  if (drawer->instances_length >= drawer->instances_reserved) {
    return false;
  }

  const size_t offset = drawer->instances_length*drawer->instance_size;
  glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
  glBufferSubData(GL_ARRAY_BUFFER, offset, drawer->instance_size, instance);

  ++drawer->instances_length;
  return true;
}

void loshader_instanced_drawer_draw(
    const loshader_instanced_drawer_t* drawer, size_t primitives) {
  assert(drawer != NULL);

  if (primitives == 0 || drawer->instances_length == 0) return;

  glUseProgram(drawer->prog);
  glBindVertexArray(drawer->vao);

  loshader_uniblock_bind(drawer->uniblock, UNIBLOCK_INDEX_);

  glDrawArraysInstanced(GL_TRIANGLES, 0, primitives, drawer->instances_length);
}
