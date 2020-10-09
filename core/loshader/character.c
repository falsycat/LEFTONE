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

#include "./instanced.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/character.vshader.h"
#include "core/loshader/anysrc/character.fshader.h"

#define VSHADER_IN_CHARACTER_ID_   0
#define VSHADER_IN_FROM_MOTION_ID_ 1
#define VSHADER_IN_TO_MOTION_ID_   2
#define VSHADER_IN_MOTION_TIME_    3
#define VSHADER_IN_MARKER_         4
#define VSHADER_IN_MARKER_OFFSET_  5
#define VSHADER_IN_POS_            6
#define VSHADER_IN_SIZE_           7
#define VSHADER_IN_COLOR_          8

#define PRIMITIVE_COUNT_ 54

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

void loshader_character_drawer_initialize(
    loshader_character_drawer_t* drawer, const loshader_uniblock_t* uniblock) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);

  gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,     sizeof(loshader_header_shader_),
      loshader_character_vshader_, sizeof(loshader_character_vshader_),
      loshader_character_fshader_, sizeof(loshader_character_fshader_));

  loshader_instanced_drawer_initialize(
      &drawer->super,
      prog,
      uniblock,
      sizeof(loshader_character_drawer_internal_instance_t));

  glBindVertexArray(drawer->super.vao);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->super.instances);

# define enable_(index, var, dim, type) do {  \
    glEnableVertexAttribArray(index);  \
    glVertexAttribPointer(  \
        index, dim, type, GL_FALSE,  \
        sizeof(loshader_character_drawer_internal_instance_t),  \
        NULL + offsetof(loshader_character_drawer_internal_instance_t, var));  \
    glVertexAttribDivisor(index, 1);  \
  } while (0)

  enable_(VSHADER_IN_CHARACTER_ID_,   character_id,   1, GL_UNSIGNED_SHORT);
  enable_(VSHADER_IN_FROM_MOTION_ID_, from_motion_id, 1, GL_UNSIGNED_SHORT);
  enable_(VSHADER_IN_TO_MOTION_ID_,   to_motion_id,   1, GL_UNSIGNED_SHORT);
  enable_(VSHADER_IN_MOTION_TIME_,    motion_time,    1, GL_FLOAT);
  enable_(VSHADER_IN_MARKER_,         marker,         1, GL_FLOAT);
  enable_(VSHADER_IN_MARKER_OFFSET_,  marker_offset,  2, GL_FLOAT);

  enable_(VSHADER_IN_POS_,   pos,   2, GL_FLOAT);
  enable_(VSHADER_IN_SIZE_,  size,  2, GL_FLOAT);
  enable_(VSHADER_IN_COLOR_, color, 4, GL_FLOAT);

# undef enable_
}

void loshader_character_drawer_add_instance(
    loshader_character_drawer_t*                drawer,
    const loshader_character_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  const loshader_character_drawer_internal_instance_t insta = {
    .character_id   = instance->character_id,
    .from_motion_id = instance->motion.from,
    .to_motion_id   = instance->motion.to,
    .motion_time    = instance->motion.time,
    .marker         = instance->marker,
    .marker_offset  = instance->marker_offset,
    .pos   = instance->pos,
    .size  = instance->size,
    .color = instance->color,
  };
  if (!loshader_instanced_drawer_add_instance(&drawer->super, &insta)) {
    fprintf(stderr, "character drawer overflow\n");
    abort();
  }
}

void loshader_character_drawer_draw(const loshader_character_drawer_t* drawer) {
  assert(drawer != NULL);

  loshader_instanced_drawer_draw(&drawer->super, PRIMITIVE_COUNT_);
}
