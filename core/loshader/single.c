#include "./single.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/program.h"

#include "./uniblock.h"

#define UNIBLOCK_INDEX_ 0
#define PARAM_INDEX_    1

void loshader_single_drawer_initialize(
    loshader_single_drawer_t*  drawer,
    gleasy_program_t           prog,
    const loshader_uniblock_t* uniblock,
    size_t                     param_size) {
  assert(drawer != NULL);
  assert(uniblock != NULL);

  const GLuint uniblock_index = glGetUniformBlockIndex(prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(prog, uniblock_index, UNIBLOCK_INDEX_);

  if (param_size > 0) {
    const GLuint param_index = glGetUniformBlockIndex(prog, "param");
    assert(glGetError() == GL_NO_ERROR);
    glUniformBlockBinding(prog, param_index, PARAM_INDEX_);
  }

  *drawer = (typeof(*drawer)) {
    .prog       = prog,
    .uniblock   = uniblock,
    .param_size = param_size,
  };

  if (param_size > 0) {
    glGenBuffers(1, &drawer->param);
    glBindBuffer(GL_UNIFORM_BUFFER, drawer->param);
    glBufferData(GL_UNIFORM_BUFFER, param_size, NULL, GL_DYNAMIC_DRAW);
  }
}

void loshader_single_drawer_deinitialize(loshader_single_drawer_t* drawer) {
  assert(drawer != NULL);

  if (drawer->param_size > 0) {
    glDeleteBuffers(1, &drawer->param);
  }
  glDeleteProgram(drawer->prog);
}

void loshader_single_drawer_set_param(
    loshader_single_drawer_t* drawer, const void* ptr) {
  assert(drawer != NULL);
  assert(drawer->param_size > 0);

  assert(ptr != NULL);

  glBindBuffer(GL_UNIFORM_BUFFER, drawer->param);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, drawer->param_size, ptr);
}

void loshader_single_drawer_draw(
    const loshader_single_drawer_t* drawer, size_t primitives) {
  assert(drawer != NULL);

  if (primitives == 0) return;

  glUseProgram(drawer->prog);
  loshader_single_drawer_draw_without_use_program(drawer, primitives);
}

void loshader_single_drawer_draw_without_use_program(
    const loshader_single_drawer_t* drawer, size_t primitives) {
  assert(drawer != NULL);

  if (primitives == 0) return;

  loshader_uniblock_bind(drawer->uniblock, UNIBLOCK_INDEX_);

  if (drawer->param_size > 0) {
    glBindBufferBase(GL_UNIFORM_BUFFER, PARAM_INDEX_, drawer->param);
  }

  glDrawArrays(GL_TRIANGLES, 0, primitives);
}
