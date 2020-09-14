#include "./character.h"

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
#include "anysrc/character.vshader.h"
#include "anysrc/character.fshader.h"

#define LOSHADER_CHARACTER_VSHADER_IN_CHARACTER_ID   0
#define LOSHADER_CHARACTER_VSHADER_IN_FROM_MOTION_ID 1
#define LOSHADER_CHARACTER_VSHADER_IN_TO_MOTION_ID   2
#define LOSHADER_CHARACTER_VSHADER_IN_MOTION_TIME    3
#define LOSHADER_CHARACTER_VSHADER_IN_MARKER         4
#define LOSHADER_CHARACTER_VSHADER_IN_MARKER_OFFSET  5
#define LOSHADER_CHARACTER_VSHADER_IN_POS            6
#define LOSHADER_CHARACTER_VSHADER_IN_SIZE           7
#define LOSHADER_CHARACTER_VSHADER_IN_COLOR          8

struct loshader_character_drawer_t {
  const loshader_character_program_t* prog;

  const loshader_uniblock_t* uniblock;

  GLuint vao;

  gleasy_buffer_array_t instances;
  size_t                instances_reserved;
  size_t                instances_length;
};

#pragma pack(push, 1)
typedef struct {
  uint16_t character_id;

  uint16_t from_motion_id;
  uint16_t to_motion_id;
  float    motion_time;

  float  marker;
  vec2_t marker_offset;

  vec2_t pos;
  vec2_t size;
  vec4_t color;
} loshader_character_drawer_internal_instance_t;
#pragma pack(pop)

#define LOSHADER_CHARACTER_UNIBLOCK_INDEX 0

#define LOSHADER_CHARACTER_PRIMITIVE_COUNT 54

static void loshader_character_program_setup_vao_(
    const loshader_character_program_t* prog, gleasy_buffer_array_t instances) {
  assert(prog      != NULL);
  assert(instances != 0);

  glBindBuffer(GL_ARRAY_BUFFER, instances);

# define enable_attrib_(NAME, name, dim, type) do {  \
    glEnableVertexAttribArray(LOSHADER_CHARACTER_VSHADER_IN_##NAME);  \
    glVertexAttribPointer(  \
        LOSHADER_CHARACTER_VSHADER_IN_##NAME, dim, type, GL_FALSE,  \
        sizeof(loshader_character_drawer_internal_instance_t),  \
        NULL + offsetof(loshader_character_drawer_internal_instance_t, name));  \
    glVertexAttribDivisor(LOSHADER_CHARACTER_VSHADER_IN_##NAME, 1);  \
  } while (0)

  enable_attrib_(CHARACTER_ID,   character_id,   1, GL_UNSIGNED_SHORT);
  enable_attrib_(FROM_MOTION_ID, from_motion_id, 1, GL_UNSIGNED_SHORT);
  enable_attrib_(TO_MOTION_ID,   to_motion_id,   1, GL_UNSIGNED_SHORT);
  enable_attrib_(MOTION_TIME,    motion_time,    1, GL_FLOAT);
  enable_attrib_(MARKER,         marker,         1, GL_FLOAT);
  enable_attrib_(MARKER_OFFSET,  marker_offset,  2, GL_FLOAT);

  enable_attrib_(POS,   pos,   2, GL_FLOAT);
  enable_attrib_(SIZE,  size,  2, GL_FLOAT);
  enable_attrib_(COLOR, color, 4, GL_FLOAT);

# undef enable_attrib_
}

void loshader_character_program_initialize(loshader_character_program_t* prog) {
  assert(prog != NULL);

  *prog = gleasy_program_new(
      loshader_header_shader_,     sizeof(loshader_header_shader_),
      loshader_character_vshader_, sizeof(loshader_character_vshader_),
      loshader_character_fshader_, sizeof(loshader_character_fshader_));

  const GLuint uniblock = glGetUniformBlockIndex(*prog, "uniblock");
  assert(glGetError() == GL_NO_ERROR);

  glUniformBlockBinding(*prog, uniblock, LOSHADER_CHARACTER_UNIBLOCK_INDEX);
}

void loshader_character_program_deinitialize(
    loshader_character_program_t* prog) {
  assert(prog != NULL);

  glDeleteProgram(*prog);
}

loshader_character_drawer_t* loshader_character_drawer_new(
    const loshader_character_program_t* prog,
    const loshader_uniblock_t*       uniblock) {
  assert(prog     != NULL);
  assert(uniblock != NULL);

  loshader_character_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {
    .prog     = prog,
    .uniblock = uniblock,
  };

  glGenBuffers(1, &drawer->instances);
  glCreateVertexArrays(1, &drawer->vao);
  glBindVertexArray(drawer->vao);
  loshader_character_program_setup_vao_(drawer->prog, drawer->instances);

  return drawer;
}

void loshader_character_drawer_delete(loshader_character_drawer_t* drawer) {
  if (drawer == NULL) return;

  glDeleteBuffers(1, &drawer->instances);

  glDeleteVertexArrays(1, &drawer->vao);

  memory_delete(drawer);
}

void loshader_character_drawer_clear(
    loshader_character_drawer_t* drawer, size_t reserve) {
  assert(drawer != NULL);
  assert(reserve > 0);

  drawer->instances_length = 0;

  if (drawer->instances_reserved < reserve) {
    glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
    glBufferData(GL_ARRAY_BUFFER,
        reserve * sizeof(loshader_character_drawer_internal_instance_t),
        NULL, GL_DYNAMIC_DRAW);
    drawer->instances_reserved = reserve;
  }
}

void loshader_character_drawer_add_instance(
    loshader_character_drawer_t*                drawer,
    const loshader_character_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  if (drawer->instances_length >= drawer->instances_reserved) {
    fprintf(stderr, "character drawer instance overflow\n");
    abort();
  }

  const loshader_character_drawer_internal_instance_t insta = {
    .character_id   = instance->character_id,
    .from_motion_id = instance->from_motion_id,
    .to_motion_id   = instance->to_motion_id,
    .motion_time    = instance->motion_time,
    .marker         = instance->marker,
    .marker_offset  = instance->marker_offset,
    .pos   = instance->pos,
    .size  = instance->size,
    .color = instance->color,
  };

  const size_t offset = drawer->instances_length * sizeof(insta);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
  glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(insta), &insta);

  ++drawer->instances_length;
}

void loshader_character_drawer_draw(const loshader_character_drawer_t* drawer) {
  assert(drawer != NULL);

  if (drawer->instances_length == 0) return;

  glUseProgram(*drawer->prog);
  glBindVertexArray(drawer->vao);

  loshader_uniblock_bind(drawer->uniblock, LOSHADER_CHARACTER_UNIBLOCK_INDEX);

  glDrawArraysInstanced(GL_TRIANGLES,
      0, LOSHADER_CHARACTER_PRIMITIVE_COUNT, drawer->instances_length);
}
