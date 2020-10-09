#include "./combat_ring.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <GL/glew.h>

#include "util/gleasy/program.h"
#include "util/math/algorithm.h"
#include "util/math/constant.h"
#include "util/math/vector.h"

#include "./instanced.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/combat_ring.vshader.h"
#include "core/loshader/anysrc/combat_ring.fshader.h"

#define VSHADER_IN_RANGE_  0
#define VSHADER_IN_PERIOD_ 1
#define VSHADER_IN_COLOR_  2

#define PRIMITIVE_COUNT_ 6

#pragma pack(push, 1)
typedef struct {
  float  range;
  vec2_t period;  /* x~y */
  vec4_t color;
} loshader_combat_ring_drawer_internal_instance_t;
#pragma pack(pop)

void loshader_combat_ring_drawer_initialize(
    loshader_combat_ring_drawer_t* drawer,
    const loshader_uniblock_t*     uniblock) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,       sizeof(loshader_header_shader_),
      loshader_combat_ring_vshader_, sizeof(loshader_combat_ring_vshader_),
      loshader_combat_ring_fshader_, sizeof(loshader_combat_ring_fshader_));

  loshader_instanced_drawer_initialize(
      &drawer->super,
      prog,
      uniblock,
      sizeof(loshader_combat_ring_drawer_internal_instance_t));

  glBindVertexArray(drawer->super.vao);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->super.instances);

# define enable_(index, var, dim, type) do {  \
    glEnableVertexAttribArray(index);  \
    glVertexAttribPointer(  \
        index, dim, type, GL_FALSE,  \
        sizeof(loshader_combat_ring_drawer_internal_instance_t),  \
        NULL +  \
        offsetof(loshader_combat_ring_drawer_internal_instance_t, var));  \
    glVertexAttribDivisor(index, 1);  \
  } while (0)

  enable_(VSHADER_IN_RANGE_,  range,  1, GL_FLOAT);
  enable_(VSHADER_IN_PERIOD_, period, 2, GL_FLOAT);
  enable_(VSHADER_IN_COLOR_,  color,  4, GL_FLOAT);

# undef enable_
}

void loshader_combat_ring_drawer_add_instance(
    loshader_combat_ring_drawer_t*                drawer,
    const loshader_combat_ring_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  const loshader_combat_ring_drawer_internal_instance_t insta = {
    .range  = instance->range,
    .period = vec2(instance->start, instance->end),
    .color  = instance->color,
  };
  if (!loshader_instanced_drawer_add_instance(&drawer->super, &insta)) {
    fprintf(stderr, "combat ring drawer overflow\n");
    abort();
  }
}

void loshader_combat_ring_drawer_draw(
    const loshader_combat_ring_drawer_t* drawer) {
  assert(drawer != NULL);

  loshader_instanced_drawer_draw(&drawer->super, PRIMITIVE_COUNT_);
}
