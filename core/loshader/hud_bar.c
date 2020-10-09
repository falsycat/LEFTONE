#include "./hud_bar.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./instanced.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/hud_bar.vshader.h"
#include "core/loshader/anysrc/hud_bar.fshader.h"

#define VSHADER_IN_POS_        0
#define VSHADER_IN_SIZE_       1
#define VSHADER_IN_BGCOLOR_    2
#define VSHADER_IN_FGCOLOR_    3
#define VSHADER_IN_VALUE_      4
#define VSHADER_IN_PREV_VALUE_ 5

#define PRIMITIVE_COUNT_ 18

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

void loshader_hud_bar_drawer_initialize(
    loshader_hud_bar_drawer_t* drawer,
    const loshader_uniblock_t* uniblock) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,   sizeof(loshader_header_shader_),
      loshader_hud_bar_vshader_, sizeof(loshader_hud_bar_vshader_),
      loshader_hud_bar_fshader_, sizeof(loshader_hud_bar_fshader_));

  loshader_instanced_drawer_initialize(
      &drawer->super,
      prog,
      uniblock,
      sizeof(loshader_hud_bar_drawer_internal_instance_t));

  glBindVertexArray(drawer->super.vao);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->super.instances);

# define enable_(index, var, dim, type) do {  \
    glEnableVertexAttribArray(index);  \
    glVertexAttribPointer(  \
        index, dim, type, GL_FALSE,  \
        sizeof(loshader_hud_bar_drawer_internal_instance_t),  \
        NULL + offsetof(loshader_hud_bar_drawer_internal_instance_t, var));  \
    glVertexAttribDivisor(index, 1);  \
  } while (0)

  enable_(VSHADER_IN_POS_,        pos,        2, GL_FLOAT);
  enable_(VSHADER_IN_SIZE_,       size,       2, GL_FLOAT);
  enable_(VSHADER_IN_BGCOLOR_,    bgcolor,    4, GL_FLOAT);
  enable_(VSHADER_IN_FGCOLOR_,    fgcolor,    4, GL_FLOAT);
  enable_(VSHADER_IN_VALUE_,      value,      1, GL_FLOAT);
  enable_(VSHADER_IN_PREV_VALUE_, prev_value, 1, GL_FLOAT);

# undef enable_
}

void loshader_hud_bar_drawer_add_instance(
    loshader_hud_bar_drawer_t*                drawer,
    const loshader_hud_bar_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  const loshader_hud_bar_drawer_internal_instance_t insta = {
    .pos        = instance->pos,
    .size       = instance->size,
    .bgcolor    = instance->bgcolor,
    .fgcolor    = instance->fgcolor,
    .value      = instance->value,
    .prev_value = instance->prev_value,
  };
  if (!loshader_instanced_drawer_add_instance(&drawer->super, &insta)) {
    fprintf(stderr, "hud bar drawer instance overflow\n");
    abort();
  }
}

void loshader_hud_bar_drawer_draw(const loshader_hud_bar_drawer_t* drawer) {
  assert(drawer != NULL);

  loshader_instanced_drawer_draw(&drawer->super, PRIMITIVE_COUNT_);
}
