#include "./linear.h"

#include <stdbool.h>

#include "util/coly2d/shape.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/loshader/bullet.h"

#include "./base.h"

#define FADE_DURATION_ 200

static bool lobullet_linear_update_(lobullet_base_t* base) {
  assert(base != NULL);

  const uint64_t t  = base->ticker->time - base->param.since;
  const float    tf = t/1000.f;

  if (t >= base->param.duration) {
    return false;
  }
  const uint64_t rt    = base->param.duration - t;
  const float    alpha = rt < FADE_DURATION_? rt*1.f/FADE_DURATION_: 1;

  /* ---- movement ---- */
  vec2_t v1;
  vec2_mul(&v1, &base->param.velocity, tf);
  vec2_t v2;
  vec2_mul(&v2, &base->param.acceleration, tf*tf/2);

  base->super.super.pos = base->param.basepos;
  vec2_addeq(&base->super.super.pos.fract, &v1);
  vec2_addeq(&base->super.super.pos.fract, &v2);
  locommon_position_reduce(&base->super.super.pos);

  /* ---- velocity ---- */
  vec2_mul(&base->super.velocity, &base->param.acceleration, tf);
  vec2_addeq(&base->super.velocity, &base->param.velocity);

  /* ---- parameters ---- */
  const float angle = base->param.angle + base->param.angular_velocity*tf;
  base->cache = (typeof(base->cache)) {
    .toxic     = true,
    .knockback = base->param.knockback,

    .instance = {
      .size  = base->param.size,
      .theta = angle,
      .color = base->param.color,
      .time  = alpha,
    },
  };
  base->super.shape = (coly2d_shape_t) {
    .size  = base->param.size,
    .angle = angle,
  };
  return true;
}

bool lobullet_linear_circle_update(lobullet_base_t* base) {
  assert(base != NULL);

  if (!lobullet_linear_update_(base)) return false;
  base->cache.instance.bullet_id = LOSHADER_BULLET_ID_LIGHT;
  base->super.shape.type         = COLY2D_SHAPE_TYPE_ELLIPSE;

  return true;
}
bool lobullet_linear_triangle_update(lobullet_base_t* base) {
  assert(base != NULL);

  if (!lobullet_linear_update_(base)) return false;
  base->cache.instance.bullet_id = LOSHADER_BULLET_ID_TRIANGLE;
  base->super.shape.type         = COLY2D_SHAPE_TYPE_TRIANGLE;

  return true;
}
bool lobullet_linear_square_update(lobullet_base_t* base) {
  assert(base != NULL);

  if (!lobullet_linear_update_(base)) return false;
  base->cache.instance.bullet_id = LOSHADER_BULLET_ID_SQUARE;
  base->super.shape.type         = COLY2D_SHAPE_TYPE_RECT;

  return true;
}

void lobullet_linear_build_(
    lobullet_base_t*             base,
    const lobullet_base_param_t* param) {
  assert(base  != NULL);
  assert(param != NULL);

  base->super.super.pos = param->basepos;
  base->param           = *param;
  base->param.since     = base->ticker->time;
}
