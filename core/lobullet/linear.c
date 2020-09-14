#include "./linear.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/coly2d/shape.h"
#include "util/math/constant.h"
#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/msgpack.h"
#include "core/locommon/position.h"
#include "core/loeffect/effect.h"
#include "core/loentity/entity.h"

#include "./base.h"
#include "./misc.h"

#define LOBULLET_LINEAR_PARAM_TO_PACK_EACH_(PROC) do {  \
  PROC("owner",        owner);  \
  PROC("pos",          pos);  \
  PROC("size",         size);  \
  PROC("velocity",     velocity);  \
  PROC("acceleration", acceleration);  \
  PROC("color",        color);  \
  PROC("duration",     duration);  \
  PROC("knockback",    knockback);  \
  PROC("effect",       effect);  \
  PROC("since",        since);  \
} while (0)
#define LOBULLET_LINEAR_PARAM_TO_PACK_COUNT 10

_Static_assert(sizeof(lobullet_linear_param_t) <= LOBULLET_BASE_DATA_MAX_SIZE);

static bool lobullet_linear_update_(lobullet_base_t* base) {
  assert(base != NULL);

  const lobullet_linear_param_t* p = (typeof(p)) base->data;

  const float t = (base->ticker->time - p->since)/1000.f;

  base->super.owner = p->owner;

  /* ---- movement ---- */
  vec2_t v1;
  vec2_mul(&v1, &p->velocity, t);

  vec2_t v2;
  vec2_mul(&v2, &p->acceleration, t*t/2);

  base->super.super.pos = p->pos;
  vec2_addeq(&base->super.super.pos.fract, &v1);
  vec2_addeq(&base->super.super.pos.fract, &v2);
  locommon_position_reduce(&base->super.super.pos);

  /* ---- velocity ---- */
  vec2_mul(&base->super.velocity, &p->acceleration, t);
  vec2_addeq(&base->super.velocity, &p->velocity);

  /* ---- angle ---- */
  const float theta = vec2_pow_length(&base->super.velocity) != 0?
      atan2f(base->super.velocity.y, base->super.velocity.x): 0;
  base->super.shape.size  = p->size;
  base->super.shape.angle = theta;

  /* ---- parameter update ---- */
  base->cache.toxic  = true;
  base->cache.effect = p->effect;

  base->cache.knockback = (typeof(base->cache.knockback)) {
    .acceleration = p->knockback,
    .algorithm    = LOBULLET_BASE_KNOCKBACK_ALGORITHM_VELOCITY,
  };

  return p->since + p->duration > base->ticker->time;
}

bool lobullet_linear_param_valid(const lobullet_linear_param_t* param) {
  return
      param != NULL &&
      locommon_position_valid(&param->pos) &&
      vec2_valid(&param->size) &&
      vec2_valid(&param->velocity) &&
      vec2_valid(&param->acceleration) &&
      param->duration > 0;
}
void lobullet_linear_param_pack(
    const lobullet_linear_param_t* p, msgpack_packer* packer) {
  assert(lobullet_linear_param_valid(p));
  assert(packer != NULL);

  msgpack_pack_map(packer, LOBULLET_LINEAR_PARAM_TO_PACK_COUNT);

# define pack_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &p->var);  \
  } while (0)

  LOBULLET_LINEAR_PARAM_TO_PACK_EACH_(pack_);

# undef pack_
}
bool lobullet_linear_param_unpack(
    lobullet_linear_param_t* p, const msgpack_object* obj) {
  assert(p != NULL);

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define unpack_(name, var) do {  \
    if (!LOCOMMON_MSGPACK_UNPACK_ANY(item_(name), &p->var)) {  \
      return false;  \
    }  \
  } while (0)

  LOBULLET_LINEAR_PARAM_TO_PACK_EACH_(unpack_);
  return lobullet_linear_param_valid(p);

# undef unpack_

# undef item_
}

void lobullet_linear_build(
    lobullet_base_t*               base,
    lobullet_type_t                type,
    const lobullet_linear_param_t* param) {
  assert(base != NULL);
  assert(lobullet_linear_param_valid(param));

  base->type = type;

  lobullet_linear_param_t* p = (typeof(p)) base->data;
  *p = *param;
  p->since = base->ticker->time;
}

bool lobullet_linear_light_update(lobullet_base_t* base) {
  assert(base != NULL);

  static const uint64_t fadedur = 500;

  if (!lobullet_linear_update_(base)) return false;

  const lobullet_linear_param_t* p = (typeof(p)) base->data;

  /* ---- calculation ---- */
  vec2_t size = p->size;
  vec2_muleq(&size, 1.2f);

  float alpha = 1;
  const uint64_t remain = p->duration - (base->ticker->time - p->since);
  if (remain <= fadedur) alpha = remain*1.f / fadedur;

  /* ---- apply result ---- */
  base->super.shape.type = COLY2D_SHAPE_TYPE_ELLIPSE;

  base->cache.instance = (loshader_bullet_drawer_instance_t) {
    .bullet_id = LOSHADER_BULLET_ID_LIGHT,
    .size      = size,
    .theta     = base->super.shape.angle,
    .color     = p->color,
  };
  base->cache.instance.color.w = alpha;
  return true;
}

bool lobullet_linear_triangle_update(lobullet_base_t* base) {
  assert(base != NULL);

  if (!lobullet_linear_update_(base)) return false;

  const lobullet_linear_param_t* p = (typeof(p)) base->data;

  /* ---- calculation ---- */
  vec2_t size = p->size;
  size.x *= 1-(1-cos(MATH_PI/3))/2;
  size.y *= 1-(1-sin(MATH_PI/3))/2;

  /* ---- apply result ---- */
  base->super.shape.type = COLY2D_SHAPE_TYPE_TRIANGLE;

  base->cache.instance = (loshader_bullet_drawer_instance_t) {
    .bullet_id = LOSHADER_BULLET_ID_TRIANGLE,
    .size      = size,
    .theta     = base->super.shape.angle,
    .color     = p->color,
    .time      = 1,
  };
  return true;
}
