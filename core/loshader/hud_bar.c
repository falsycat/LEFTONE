#include "./hud_bar.h"

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
#include "anysrc/hud_bar.vshader.h"
#include "anysrc/hud_bar.fshader.h"

#define LOSHADER_HUD_BAR_VSHADER_IN_POS        0
#define LOSHADER_HUD_BAR_VSHADER_IN_SIZE       1
#define LOSHADER_HUD_BAR_VSHADER_IN_BGCOLOR    2
#define LOSHADER_HUD_BAR_VSHADER_IN_FGCOLOR    3
#define LOSHADER_HUD_BAR_VSHADER_IN_VALUE      4
#define LOSHADER_HUD_BAR_VSHADER_IN_PREV_VALUE 5

struct loshader_hud_bar_drawer_t {
  const loshader_hud_bar_program_t* prog;
  const loshader_uniblock_t*        uniblock;

  GLuint vao;

  gleasy_buffer_array_t instances;
  size_t                instances_reserved;
  size_t                instances_length;
};

#pragma pack(push, 1)
typedef struct {
  vec2_t pos;
  vec2_t size;

  vec4_t bgcolor;
  vec4_t fgcolor;

  float value;
  float prev_value;
} loshader_hud_bar_drawer_internal_instance_t;
#pragma pack(pop)

#define LOSHADER_HUD_BAR_UNIBLOCK_INDEX 0

#define LOSHADER_HUD_BAR_PRIMITIVE_COUNT 18

static void loshader_hud_bar_program_setup_vao_(
    gleasy_buffer_array_t instances) {
  assert(instances != 0);

  glBindBuffer(GL_ARRAY_BUFFER, instances);

# define enable_attrib_(NAME, name, dim, type) do {  \
    glEnableVertexAttribArray(LOSHADER_HUD_BAR_VSHADER_IN_##NAME);  \
    glVertexAttribPointer(  \
        LOSHADER_HUD_BAR_VSHADER_IN_##NAME, dim, type, GL_FALSE,  \
        sizeof(loshader_hud_bar_drawer_internal_instance_t),  \
        NULL + offsetof(loshader_hud_bar_drawer_internal_instance_t, name));  \
    glVertexAttribDivisor(LOSHADER_HUD_BAR_VSHADER_IN_##NAME, 1);  \
  } while (0)

  enable_attrib_(POS,        pos,        2, GL_FLOAT);
  enable_attrib_(SIZE,       size,       2, GL_FLOAT);
  enable_attrib_(BGCOLOR,    bgcolor,    4, GL_FLOAT);
  enable_attrib_(FGCOLOR,    fgcolor,    4, GL_FLOAT);
  enable_attrib_(VALUE,      value,      1, GL_FLOAT);
  enable_attrib_(PREV_VALUE, prev_value, 1, GL_FLOAT);

# undef enable_attrib_
}

void loshader_hud_bar_program_initialize(loshader_hud_bar_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,   sizeof(loshader_header_shader_),
      loshader_hud_bar_vshader_, sizeof(loshader_hud_bar_vshader_),
      loshader_hud_bar_fshader_, sizeof(loshader_hud_bar_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);
  glUniformBlockBinding(*prog, uniblock, LOSHADER_HUD_BAR_UNIBLOCK_INDEX);
}

void loshader_hud_bar_program_deinitialize(loshader_hud_bar_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_hud_bar_drawer_t* loshader_hud_bar_drawer_new(
    const loshader_hud_bar_program_t* prog,
    const loshader_uniblock_t*    uniblock) {
  assert(prog     != NULL);
  assert(uniblock != NULL);

  loshader_hud_bar_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };

  glCreateVertexArrays(1, &drawer->vao);
  glBindVertexArray(drawer->vao);

  glGenBuffers(1, &drawer->instances);
  loshader_hud_bar_program_setup_vao_(drawer->instances);

  return drawer;
}

void loshader_hud_bar_drawer_delete(loshader_hud_bar_drawer_t* drawer) {
  if (drawer == NULL) return;

  glDeleteBuffers(1, &drawer->instances);

  glDeleteVertexArrays(1, &drawer->vao);

  memory_delete(drawer);
}

void loshader_hud_bar_drawer_clear(
    loshader_hud_bar_drawer_t* drawer, size_t reserve) {
  assert(drawer != NULL);
  assert(reserve > 0);

  drawer->instances_length = 0;

  if (drawer->instances_reserved < reserve) {
    glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(loshader_hud_bar_drawer_internal_instance_t) * reserve,
        NULL, GL_DYNAMIC_DRAW);
    drawer->instances_reserved = reserve;
  }
}

void loshader_hud_bar_drawer_add_instance(
    loshader_hud_bar_drawer_t*                drawer,
    const loshader_hud_bar_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  if (drawer->instances_length >= drawer->instances_reserved) {
    fprintf(stderr, "hud bar drawer instance overflow\n");
    abort();
  }

  const loshader_hud_bar_drawer_internal_instance_t insta = {
    .pos        = instance->pos,
    .size       = instance->size,
    .bgcolor    = instance->bgcolor,
    .fgcolor    = instance->fgcolor,
    .value      = instance->value,
    .prev_value = instance->prev_value,
  };

  const size_t offset = drawer->instances_length * sizeof(insta);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
  glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(insta), &insta);

  ++drawer->instances_length;
}

void loshader_hud_bar_drawer_draw(const loshader_hud_bar_drawer_t* drawer) {
  assert(drawer != NULL);

  if (drawer->instances_length == 0) return;

  glUseProgram(*drawer->prog);
  glBindVertexArray(drawer->vao);

  loshader_uniblock_bind(drawer->uniblock, LOSHADER_HUD_BAR_UNIBLOCK_INDEX);

  glDrawArraysInstanced(GL_TRIANGLES,
      0, LOSHADER_HUD_BAR_PRIMITIVE_COUNT, drawer->instances_length);
}
