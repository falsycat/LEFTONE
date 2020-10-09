#include "./fog.h"

#include <assert.h>

#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "core/locommon/position.h"

#include "./single.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/fog.vshader.h"
#include "core/loshader/anysrc/fog.fshader.h"

#define PRIMITIVE_COUNT_ 6

#pragma pack(push, 1)
typedef struct {
  float type;
  float prev_type;
  float transition;

  float  bounds_fog;
  vec4_t bounds_pos;
  vec2_t bounds_size;
} loshader_fog_drawer_internal_param_t;
#pragma pack(pop)

_Static_assert(
    sizeof(float)*10 ==
    sizeof(loshader_fog_drawer_internal_param_t),
    "recheck the type has no padding");

void loshader_fog_drawer_initialize(
    loshader_fog_drawer_t*     drawer,
    const loshader_uniblock_t* uniblock) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_, sizeof(loshader_header_shader_),
      loshader_fog_vshader_,   sizeof(loshader_fog_vshader_),
      loshader_fog_fshader_,   sizeof(loshader_fog_fshader_));

  loshader_single_drawer_initialize(
      &drawer->super,
      prog,
      uniblock,
      sizeof(loshader_fog_drawer_internal_param_t));
}

void loshader_fog_drawer_set_param(
    loshader_fog_drawer_t*             drawer,
    const loshader_fog_drawer_param_t* param) {
  assert(drawer != NULL);
  assert(param  != NULL);

  const loshader_fog_drawer_internal_param_t p = {
    .type        = param->type,
    .prev_type   = param->prev_type,
    .transition  = param->transition,
    .bounds_fog  = param->bounds_fog,
    .bounds_pos  = vec4(
        param->bounds_pos.chunk.x,
        param->bounds_pos.chunk.y,
        param->bounds_pos.fract.x,
        param->bounds_pos.fract.y),
    .bounds_size = param->bounds_size,
  };
  loshader_single_drawer_set_param(&drawer->super, &p);
}

void loshader_fog_drawer_draw(const loshader_fog_drawer_t* drawer) {
  assert(drawer != NULL);

  loshader_single_drawer_draw(&drawer->super, PRIMITIVE_COUNT_);
}
