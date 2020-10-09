#include "./menu_stance.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./instanced.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/menu_stance.vshader.h"
#include "core/loshader/anysrc/menu_stance.fshader.h"

#define VSHADER_IN_ID_        0
#define VSHADER_IN_POS_       1
#define VSHADER_IN_SIZE_      2
#define VSHADER_IN_ALPHA_     3

#define PRIMITIVE_COUNT_ 60

#pragma pack(push, 1)
typedef struct {
  uint8_t id;
  vec2_t  pos;
  vec2_t  size;
  float   alpha;
} loshader_menu_stance_drawer_internal_instance_t;
#pragma pack(pop)

void loshader_menu_stance_drawer_initialize(
    loshader_menu_stance_drawer_t* drawer,
    const loshader_uniblock_t*     uniblock) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,       sizeof(loshader_header_shader_),
      loshader_menu_stance_vshader_, sizeof(loshader_menu_stance_vshader_),
      loshader_menu_stance_fshader_, sizeof(loshader_menu_stance_fshader_));

  loshader_instanced_drawer_initialize(
      &drawer->super,
      prog,
      uniblock,
      sizeof(loshader_menu_stance_drawer_internal_instance_t));

  glBindVertexArray(drawer->super.vao);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->super.instances);

# define enable_(index, var, dim, type) do {  \
    glEnableVertexAttribArray(index);  \
    glVertexAttribPointer(  \
        index, dim, type, GL_FALSE,  \
        sizeof(loshader_menu_stance_drawer_internal_instance_t),  \
        NULL + offsetof(loshader_menu_stance_drawer_internal_instance_t, var));  \
    glVertexAttribDivisor(index, 1);  \
  } while (0)

  enable_(VSHADER_IN_ID_,    id,    1, GL_UNSIGNED_BYTE);
  enable_(VSHADER_IN_POS_,   pos,   2, GL_FLOAT);
  enable_(VSHADER_IN_SIZE_,  size,  2, GL_FLOAT);
  enable_(VSHADER_IN_ALPHA_, alpha, 1, GL_FLOAT);

# undef enable_
}

void loshader_menu_stance_drawer_add_instance(
    loshader_menu_stance_drawer_t*                drawer,
    const loshader_menu_stance_drawer_instance_t* instance) {
  assert(drawer   != NULL);
  assert(instance != NULL);

  const loshader_menu_stance_drawer_internal_instance_t insta = {
    .id        = instance->id,
    .pos       = instance->pos,
    .size      = instance->size,
    .alpha     = instance->alpha,
  };
  if (!loshader_instanced_drawer_add_instance(&drawer->super, &insta)) {
    fprintf(stderr, "menu stance drawer overflow\n");
    abort();
  }
}

void loshader_menu_stance_drawer_draw(
    const loshader_menu_stance_drawer_t* drawer) {
  assert(drawer != NULL);

  loshader_instanced_drawer_draw(&drawer->super, PRIMITIVE_COUNT_);
}
