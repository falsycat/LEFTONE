#include "./easing.h"

#include <assert.h>
#include <stddef.h>

#include "util/math/algorithm.h"

void locommon_easing_linear_float(float* v, float ed, float delta) {
  assert(v != NULL);
  assert(MATH_FLOAT_VALID(*v));
  assert(MATH_FLOAT_VALID(delta));
  assert(delta >= 0);

  const float flag = MATH_SIGN(ed - *v);
  *v += flag * delta;
  if ((ed-*v)*flag < 0) *v = ed;
}

void locommon_easing_smooth_float(float* v, float ed, float delta) {
  assert(v != NULL);
  assert(MATH_FLOAT_VALID(*v));
  assert(MATH_FLOAT_VALID(delta));
  assert(delta >= 0);

  *v += (ed - *v) * MATH_MIN(delta, 1);
}

void locommon_easing_smooth_position(
    locommon_position_t* pos, const locommon_position_t* ed, float delta) {
  assert(locommon_position_valid(pos));
  assert(locommon_position_valid(ed));
  assert(MATH_FLOAT_VALID(delta));

  vec2_t diff;
  locommon_position_sub(&diff, ed, pos);
  vec2_muleq(&diff, MATH_MIN(delta, 1));

  vec2_addeq(&pos->fract, &diff);
  locommon_position_reduce(pos);
}
