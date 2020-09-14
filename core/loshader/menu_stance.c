#include "./menu_stance.h"

#include <assert.h>
#include <stddef.h>
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
#include "anysrc/menu_stance.vshader.h"
#include "anysrc/menu_stance.fshader.h"

#define LOSHADER_MENU_STANCE_VSHADER_IN_ID        0
#define LOSHADER_MENU_STANCE_VSHADER_IN_POS       1
#define LOSHADER_MENU_STANCE_VSHADER_IN_SIZE      2
#define LOSHADER_MENU_STANCE_VSHADER_IN_ALPHA     3
#define LOSHADER_MENU_STANCE_VSHADER_IN_HIGHLIGHT 4

struct loshader_menu_stance_drawer_t {
  const loshader_menu_stance_program_t* prog;
  const loshader_uniblock_t*            uniblock;

  GLuint vao;

  gleasy_buffer_array_t instances;
  size_t                instances_length;
  size_t                instances_reserved;
};

#pragma pack(push, 1)
typedef struct {
  uint8_t id;
  vec2_t  pos;
  vec2_t  size;
  float   alpha;
  float   highlight;
} loshader_menu_stance_drawer_internal_instance_t;
#pragma pack(pop)

#define LOSHADER_MENU_STANCE_UNIBLOCK_INDEX  0
#define LOSHADER_MENU_STANCE_PRIMITIVE_COUNT 60

static void loshader_menu_stance_program_setup_vao_(
    gleasy_buffer_array_t instances) {
  assert(instances != 0);

  glBindBuffer(GL_ARRAY_BUFFER, instances);

# define enable_attrib_(NAME, name, dim, type) do {  \
    glEnableVertexAttribArray(LOSHADER_MENU_STANCE_VSHADER_IN_##NAME);  \
    glVertexAttribPointer(  \
        LOSHADER_MENU_STANCE_VSHADER_IN_##NAME, dim, type, GL_FALSE,  \
        sizeof(loshader_menu_stance_drawer_internal_instance_t),  \
        NULL + offsetof(loshader_menu_stance_drawer_internal_instance_t, name));  \
    glVertexAttribDivisor(LOSHADER_MENU_STANCE_VSHADER_IN_##NAME, 1);  \
  } while (0)

  enable_attrib_(ID,        id,        1, GL_UNSIGNED_BYTE);
  enable_attrib_(POS,       pos,       2, GL_FLOAT);
  enable_attrib_(SIZE,      size,      2, GL_FLOAT);
  enable_attrib_(ALPHA,     alpha,     1, GL_FLOAT);
  enable_attrib_(HIGHLIGHT, highlight, 1, GL_FLOAT);

# undef enable_attrib_
}

void loshader_menu_stance_program_initialize(
    loshader_menu_stance_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,       sizeof(loshader_header_shader_),
      loshader_menu_stance_vshader_, sizeof(loshader_menu_stance_vshader_),
      loshader_menu_stance_fshader_, sizeof(loshader_menu_stance_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, uniblock, LOSHADER_MENU_STANCE_UNIBLOCK_INDEX);
}

void loshader_menu_stance_program_deinitialize(
    loshader_menu_stance_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_menu_stance_drawer_t* loshader_menu_stance_drawer_new(
    const loshader_menu_stance_program_t* prog,
    const loshader_uniblock_t*            uniblock) {
  assert(prog     != NULL);
  assert(uniblock != NULL);

  loshader_menu_stance_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };

  glCreateVertexArrays(1, &drawer->vao);
  glBindVertexArray(drawer->vao);

  glGenBuffers(1, &drawer->instances);
  loshader_menu_stance_program_setup_vao_(drawer->instances);

  return drawer;
}

void loshader_menu_stance_drawer_delete(
    loshader_menu_stance_drawer_t* drawer) {
  assert(drawer != NULL);

  glDeleteBuffers(1, &drawer->instances);

  glDeleteVertexArrays(1, &drawer->vao);

  memory_delete(drawer);
}

void loshader_menu_stance_drawer_clear(
    loshader_menu_stance_drawer_t* drawer, size_t reserve) {
  assert(drawer  != NULL);
  assert(reserve > 0);

  drawer->instances_length = 0;

  if (drawer->instances_reserved < reserve) {
    glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
    glBufferData(GL_ARRAY_BUFFER,
        reserve * sizeof(loshader_menu_stance_drawer_internal_instance_t),
        NULL, GL_DYNAMIC_DRAW);
    drawer->instances_reserved = reserve;
  }
}

void loshader_menu_stance_drawer_add_instance(
    loshader_menu_stance_drawer_t*                drawer,
    const loshader_menu_stance_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  if (drawer->instances_length >= drawer->instances_reserved) {
    fprintf(stderr, "menu stance drawer instance overflow\n");
    abort();
  }

  const loshader_menu_stance_drawer_internal_instance_t insta = {
    .id        = instance->id,
    .pos       = instance->pos,
    .size      = instance->size,
    .alpha     = instance->alpha,
    .highlight = instance->highlight,
  };

  const size_t offset = drawer->instances_length * sizeof(insta);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
  glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(insta), &insta);

  ++drawer->instances_length;
}

void loshader_menu_stance_drawer_draw(
    const loshader_menu_stance_drawer_t* drawer) {
  assert(drawer != NULL);

  if (drawer->instances_length == 0) return;

  glUseProgram(*drawer->prog);
  glBindVertexArray(drawer->vao);

  loshader_uniblock_bind(
      drawer->uniblock, LOSHADER_MENU_STANCE_UNIBLOCK_INDEX);

  glDrawArraysInstanced(GL_TRIANGLES,
      0, LOSHADER_MENU_STANCE_PRIMITIVE_COUNT, drawer->instances_length);
}
