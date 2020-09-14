#include "./physics.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/coly2d/hittest.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "./position.h"

bool locommon_physics_entity_valid(const locommon_physics_entity_t* e) {
  return
      e != NULL &&
      locommon_position_valid(&e->pos) &&
      vec2_valid(&e->velocity) &&
      vec2_valid(&e->size);
}

bool locommon_physics_solve_collision_with_fixed_one(
    locommon_physics_entity_t*       e1,
    const locommon_physics_entity_t* e2,
    float                            dt) {
  assert(locommon_physics_entity_valid(e1));
  assert(locommon_physics_entity_valid(e2));
  assert(vec2_pow_length(&e2->velocity) == 0);
  assert(MATH_FLOAT_VALID(dt));

  vec2_t size;
  vec2_add(&size, &e1->size, &e2->size);

  vec2_t pos;
  locommon_position_sub(&pos, &e1->pos, &e2->pos);
  pos.x /= size.x;
  pos.y /= size.y;

  vec2_t velocity = e1->velocity;
  velocity.x /= size.x;
  velocity.y /= size.y;

  vec2_t disp;
  vec2_mul(&disp, &velocity, dt);

  vec2_t spos;
  vec2_sub(&spos, &pos, &disp);

  static const vec2_t origin = vec2(0, 0);
  static const vec2_t sz     = vec2(1, 1);
  if (!coly2d_hittest_lineseg_and_rect(&spos, &pos, &origin, &sz)) {
    return false;
  }

  if (MATH_ABS(spos.x) < 1 && MATH_ABS(spos.y) < 1) {
    float* f = MATH_ABS(spos.x) > MATH_ABS(spos.y)? &spos.x: &spos.y;
    *f = MATH_SIGN(*f);
  }

  vec2_t vt = vec2(MATH_INF, MATH_INF);
  if (velocity.x != 0) {
    vt.x = -(MATH_SIGN(velocity.x)+spos.x) / velocity.x;
  }
  if (velocity.y != 0) {
    vt.y = -(MATH_SIGN(velocity.y)+spos.y) / velocity.y;
  }

  /* ---- simulation ---- */
  float t = MATH_MIN(vt.x, vt.y);
  if (t < 0) t = MATH_MAX(vt.x, vt.y);
  if (t < 0 || t >= dt) return false;

  vec2_t d, v = velocity, p = spos;
  vec2_mul(&d, &v, t);
  vec2_addeq(&p, &d);

  if (t == vt.x) v.x = 0;
  if (t == vt.y) v.y = 0;
  vec2_mul(&d, &v, dt-t);
  vec2_addeq(&p, &d);

  /* ---- return result ---- */
  p.x *= size.x;
  p.y *= size.y;
  v.x *= size.x;
  v.y *= size.y;

  e1->pos = e2->pos;
  vec2_addeq(&e1->pos.fract, &p);
  locommon_position_reduce(&e1->pos);

  e1->velocity = v;

  return true;
}
