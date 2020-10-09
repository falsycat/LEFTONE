#include "./backwall.h"

#include <assert.h>
#include <stddef.h>

#include "./single.h"
#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/backwall.vshader.h"
#include "core/loshader/anysrc/backwall.fshader.h"

#define PRIMITIVE_COUNT_ 6

#pragma pack(push, 1)
typedef struct {
  float type;
  float prev_type;
  float transition;
} loshader_backwall_drawer_internal_param_t;
#pragma pack(pop)

_Static_assert(
    sizeof(float)*3 ==
    sizeof(loshader_backwall_drawer_internal_param_t),
    "recheck the type has no padding");

void loshader_backwall_drawer_initialize(
    loshader_backwall_drawer_t* drawer,
    const loshader_uniblock_t*  uniblock) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,    sizeof(loshader_header_shader_),
      loshader_backwall_vshader_, sizeof(loshader_backwall_vshader_),
      loshader_backwall_fshader_, sizeof(loshader_backwall_fshader_));

  loshader_single_drawer_initialize(
      &drawer->super,
      prog,
      uniblock,
      sizeof(loshader_backwall_drawer_internal_param_t));
}

void loshader_backwall_drawer_set_param(
    loshader_backwall_drawer_t*             drawer,
    const loshader_backwall_drawer_param_t* param) {
  assert(drawer != NULL);
  assert(param  != NULL);

  const loshader_backwall_drawer_internal_param_t p = {
    .type       = param->type,
    .prev_type  = param->prev_type,
    .transition = param->transition,
  };
  loshader_single_drawer_set_param(&drawer->super, &p);
}

void loshader_backwall_drawer_draw(const loshader_backwall_drawer_t* drawer) {
  assert(drawer != NULL);

  loshader_single_drawer_draw(&drawer->super, PRIMITIVE_COUNT_);
}
