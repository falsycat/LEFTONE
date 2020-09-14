#include "./bullet.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/coly2d/shape.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "core/locommon/position.h"

#include "./decl.private.h"

bool loentity_bullet_affect(
    loentity_bullet_t* bullet, loentity_character_t* chara) {
  assert(bullet != NULL);
  assert(chara  != NULL);

  assert(bullet->vtable.affect != NULL);
  return bullet->vtable.affect(bullet, chara);
}

bool loentity_bullet_hittest(
    const loentity_bullet_t*   bullet,
    const locommon_position_t* point,
    const vec2_t*              velocity,
    float                      dt) {
  assert(bullet != NULL);
  assert(locommon_position_valid(point));
  assert(vec2_valid(velocity));
  assert(MATH_FLOAT_VALID(dt));

  vec2_t st;
  locommon_position_sub(&st, point, &bullet->super.pos);

  vec2_t ed;
  vec2_sub(&ed, velocity, &bullet->velocity);
  vec2_muleq(&ed, dt);
  vec2_addeq(&ed, &st);

  return coly2d_shape_hittest_lineseg(&bullet->shape, &st, &ed);
}
