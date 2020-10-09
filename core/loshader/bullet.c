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

#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/bullet.vshader.h"
#include "core/loshader/anysrc/bullet.fshader.h"

#define VSHADER_IN_BULLET_ID_ 0
#define VSHADER_IN_POS_       1
#define VSHADER_IN_SIZE_      2
#define VSHADER_IN_THETA_     3
#define VSHADER_IN_TIME_      4
#define VSHADER_IN_COLOR_     5

#define PRIMITIVE_COUNT_ 6

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

void loshader_bullet_drawer_initialize(
    loshader_bullet_drawer_t*  drawer,
    const loshader_uniblock_t* uniblock) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,  sizeof(loshader_header_shader_),
      loshader_bullet_vshader_, sizeof(loshader_bullet_vshader_),
      loshader_bullet_fshader_, sizeof(loshader_bullet_fshader_));

  loshader_instanced_drawer_initialize(
      &drawer->super,
      prog,
      uniblock,
      sizeof(loshader_bullet_drawer_internal_instance_t));

  glBindVertexArray(drawer->super.vao);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->super.instances);

# define enable_(index, var, dim, type) do {  \
    glEnableVertexAttribArray(index);  \
    glVertexAttribPointer(  \
        index, dim, type, GL_FALSE,  \
        sizeof(loshader_bullet_drawer_internal_instance_t),  \
        NULL + offsetof(loshader_bullet_drawer_internal_instance_t, var));  \
    glVertexAttribDivisor(index, 1);  \
  } while (0)

  enable_(VSHADER_IN_BULLET_ID_, bullet_id, 1, GL_UNSIGNED_SHORT);

  enable_(VSHADER_IN_POS_,   pos,   2, GL_FLOAT);
  enable_(VSHADER_IN_SIZE_,  size,  2, GL_FLOAT);
  enable_(VSHADER_IN_THETA_, theta, 1, GL_FLOAT);
  enable_(VSHADER_IN_TIME_,  time,  1, GL_FLOAT);

  enable_(VSHADER_IN_COLOR_, color, 4, GL_FLOAT);

# undef enable_
}

void loshader_bullet_drawer_add_instance(
    loshader_bullet_drawer_t*                drawer,
    const loshader_bullet_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  const loshader_bullet_drawer_internal_instance_t insta = {
    .bullet_id = instance->bullet_id,
    .pos       = instance->pos,
    .size      = instance->size,
    .theta     = instance->theta,
    .time      = instance->time,
    .color     = instance->color,
  };
  if (!loshader_instanced_drawer_add_instance(&drawer->super, &insta)) {
    fprintf(stderr, "bullet drawer overflow\n");
    abort();
  }
}

void loshader_bullet_drawer_draw(const loshader_bullet_drawer_t* drawer) {
  assert(drawer != NULL);

  loshader_instanced_drawer_draw(&drawer->super, PRIMITIVE_COUNT_);
}
