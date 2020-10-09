#include "./cinescope.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/vector.h"

#include "./single.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/cinescope.vshader.h"
#include "core/loshader/anysrc/cinescope.fshader.h"

#define PRIMITIVE_COUNT_ 12

#pragma pack(push, 1)
typedef struct {
  vec4_t color;
  float  size;
} loshader_cinescope_drawer_internal_param_t;
#pragma pack(pop)

_Static_assert(
    sizeof(float)*5 ==
    sizeof(loshader_cinescope_drawer_internal_param_t),
    "recheck the type has no padding");

void loshader_cinescope_drawer_initialize(
    loshader_cinescope_drawer_t* drawer,
    const loshader_uniblock_t*   uniblock) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,     sizeof(loshader_header_shader_),
      loshader_cinescope_vshader_, sizeof(loshader_cinescope_vshader_),
      loshader_cinescope_fshader_, sizeof(loshader_cinescope_fshader_));

  loshader_single_drawer_initialize(
      &drawer->super,
      prog,
      uniblock,
      sizeof(loshader_cinescope_drawer_internal_param_t));
}

void loshader_cinescope_drawer_set_param(
    loshader_cinescope_drawer_t*             drawer,
    const loshader_cinescope_drawer_param_t* param) {
  assert(drawer != NULL);
  assert(param  != NULL);

  const loshader_cinescope_drawer_internal_param_t p = {
    .size  = param->size,
    .color = param->color,
  };
  loshader_single_drawer_set_param(&drawer->super, &p);
}

void loshader_cinescope_drawer_draw(const loshader_cinescope_drawer_t* drawer) {
  assert(drawer != NULL);

  loshader_single_drawer_draw(&drawer->super, PRIMITIVE_COUNT_);
}
