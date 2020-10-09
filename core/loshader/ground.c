#include "./ground.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/math/vector.h"
#include "util/gleasy/program.h"

#include "./instanced.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/ground.vshader.h"
#include "core/loshader/anysrc/ground.fshader.h"

#define VSHADER_IN_GROUND_ID_ 0
#define VSHADER_IN_POS_       1
#define VSHADER_IN_SIZE_      2

#define PRIMITIVE_COUNT_ 6

#pragma pack(push, 1)
typedef struct {
  uint16_t ground_id;

  vec2_t pos;
  vec2_t size;
} loshader_ground_drawer_internal_instance_t;
#pragma pack(pop)

void loshader_ground_drawer_initialize(
    loshader_ground_drawer_t*  drawer,
    const loshader_uniblock_t* uniblock) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,  sizeof(loshader_header_shader_),
      loshader_ground_vshader_, sizeof(loshader_ground_vshader_),
      loshader_ground_fshader_, sizeof(loshader_ground_fshader_));

  loshader_instanced_drawer_initialize(
      &drawer->super,
      prog,
      uniblock,
      sizeof(loshader_ground_drawer_internal_instance_t));

  glBindVertexArray(drawer->super.vao);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->super.instances);

# define enable_(index, var, dim, type) do {  \
    glEnableVertexAttribArray(index);  \
    glVertexAttribPointer(  \
        index, dim, type, GL_FALSE,  \
        sizeof(loshader_ground_drawer_internal_instance_t),  \
        NULL + offsetof(loshader_ground_drawer_internal_instance_t, var));  \
    glVertexAttribDivisor(index, 1);  \
  } while (0)

  enable_(VSHADER_IN_GROUND_ID_, ground_id, 1, GL_UNSIGNED_SHORT);
  enable_(VSHADER_IN_POS_,       pos,       2, GL_FLOAT);
  enable_(VSHADER_IN_SIZE_,      size,      2, GL_FLOAT);

# undef enable_
}

void loshader_ground_drawer_add_instance(
    loshader_ground_drawer_t*                drawer,
    const loshader_ground_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  const loshader_ground_drawer_internal_instance_t inst = {
    .ground_id = instance->ground_id,
    .pos       = instance->pos,
    .size      = instance->size,
  };
  if (!loshader_instanced_drawer_add_instance(&drawer->super, &inst)) {
    fprintf(stderr, "ground drawer instance overflow\n");
    abort();
  }
}

void loshader_ground_drawer_draw(const loshader_ground_drawer_t* drawer) {
  assert(drawer != NULL);

  loshader_instanced_drawer_draw(&drawer->super, PRIMITIVE_COUNT_);
}
