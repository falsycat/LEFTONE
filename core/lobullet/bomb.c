#include "./bomb.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/coly2d/shape.h"
#include "util/math/algorithm.h"
#include "util/math/constant.h"
#include "util/math/vector.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/msgpack.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loresource/sound.h"
#include "core/loshader/bullet.h"

#include "./base.h"
#include "./misc.h"

#define LOBULLET_BOMB_PARAM_TO_PACK_EACH_(PROC) do {  \
  PROC("owner",     owner);  \
  PROC("pos",       pos);  \
  PROC("size",      size);  \
  PROC("angle",     angle);  \
  PROC("color",     color);  \
  PROC("silent",    silent);  \
  PROC("beat",      beat);  \
  PROC("step",      step);  \
  PROC("knockback", knockback);  \
  PROC("effect",    effect);  \
  PROC("since",     since);  \
} while (0)
#define LOBULLET_BOMB_PARAM_TO_PACK_COUNT 11

_Static_assert(sizeof(lobullet_bomb_param_t) <= LOBULLET_BASE_DATA_MAX_SIZE);

static bool lobullet_bomb_update_(lobullet_base_t* base) {
  assert(base != NULL);

  const lobullet_bomb_param_t* p = (typeof(p)) base->data;

  base->super.super.pos   = p->pos;
  base->super.owner       = p->owner;
  base->super.velocity    = vec2(0, 0);
  base->super.shape.size  = p->size;
  base->super.shape.angle = p->angle;

  const uint64_t st = (p->step-1) * p->beat;
  const uint64_t ed = st + 100;

  const uint64_t t  = base->ticker->time - p->since;
  const uint64_t pt =
      (int64_t) t >= base->ticker->delta? t - base->ticker->delta: 0;

  if (!p->silent && pt < st && t >= st) {
    loresource_sound_play(base->res->sound, "bomb");
  }

  base->cache.toxic  = st <= t && t < ed;
  base->cache.effect = p->effect;
  return t < p->step*p->beat;
}

bool lobullet_bomb_param_valid(const lobullet_bomb_param_t* param) {
  return
      param != NULL &&
      locommon_position_valid(&param->pos) &&
      vec2_valid(&param->size) &&
      MATH_FLOAT_VALID(param->angle) &&
      vec4_valid(&param->color) &&
      MATH_FLOAT_VALID(param->beat) &&
      param->step > 0 &&
      MATH_FLOAT_VALID(param->knockback);
}
void lobullet_bomb_param_pack(
    const lobullet_bomb_param_t* p, msgpack_packer* packer) {
  assert(lobullet_bomb_param_valid(p));
  assert(packer != NULL);

  msgpack_pack_map(packer, LOBULLET_BOMB_PARAM_TO_PACK_COUNT);

# define pack_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &p->var);  \
  } while (0)

  LOBULLET_BOMB_PARAM_TO_PACK_EACH_(pack_);

# undef pack_
}
bool lobullet_bomb_param_unpack(
    lobullet_bomb_param_t* p, const msgpack_object* obj) {
  assert(p != NULL);

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define unpack_(name, var) do {  \
    if (!LOCOMMON_MSGPACK_UNPACK_ANY(item_(name), &p->var)) {  \
      return false;  \
    }  \
  } while (0)

  LOBULLET_BOMB_PARAM_TO_PACK_EACH_(unpack_);
  return lobullet_bomb_param_valid(p);

# undef unpack_

# undef item_
}

void lobullet_bomb_build(
    lobullet_base_t*             base,
    lobullet_type_t              type,
    const lobullet_bomb_param_t* param) {
  assert(base != NULL);
  assert(lobullet_bomb_param_valid(param));

  base->type = type;

  lobullet_bomb_param_t* p = (typeof(p)) base->data;
  *p = *param;
  p->since = base->ticker->time;
}

bool lobullet_bomb_square_update(lobullet_base_t* base) {
  assert(base != NULL);

  if (!lobullet_bomb_update_(base)) return false;

  const lobullet_bomb_param_t* p = (typeof(p)) base->data;

  /* ---- calculate motion ---- */
  const float beats = (base->ticker->time - p->since) / p->beat;

  float time  = 0;
  float angle = p->angle;
  float alpha = 1;
  if (beats < p->step-1) {
    time  = beats - (int64_t) beats;
    alpha = 1-time;
    time  = time*time;
    time  = (1-time)*.05f;
  } else {
    time   = 1 - powf(1-(beats - (int64_t) beats), 2);
    angle += time * MATH_PI/4;
    time   = 1-time;
  }

  /* ---- apply motion ---- */
  base->super.shape.type  = COLY2D_SHAPE_TYPE_RECT;
  base->super.shape.angle = angle;

  base->cache.instance = (loshader_bullet_drawer_instance_t) {
    .bullet_id = LOSHADER_BULLET_ID_SQUARE,
    .size      = p->size,
    .theta     = angle,
    .color     = p->color,
    .time      = time,
  };
  base->cache.instance.color.w *= alpha;
  return true;
}

bool lobullet_bomb_triangle_update(lobullet_base_t* base) {
  assert(base != NULL);

  if (!lobullet_bomb_update_(base)) return false;

  const lobullet_bomb_param_t* p = (typeof(p)) base->data;

  /* ---- calculate motion ---- */
  const float beats = (base->ticker->time - p->since) / p->beat;

  float time  = 0;
  float alpha = 1;
  if (beats < p->step-1) {
    time  = beats - (int64_t) beats;
    alpha = 1-time;
    time  = time*time;
    time  = (1-time)*.05f;
  } else {
    time = 1 - powf(1-(beats - (int64_t) beats), 2);
    time = 1-time;
  }

  /* ---- apply motion ---- */
  base->super.shape.type  = COLY2D_SHAPE_TYPE_TRIANGLE;

  base->cache.instance = (loshader_bullet_drawer_instance_t) {
    .bullet_id = LOSHADER_BULLET_ID_TRIANGLE,
    .size      = p->size,
    .theta     = base->super.shape.angle,
    .color     = p->color,
    .time      = time,
  };
  base->cache.instance.color.w *= alpha;
  return true;
}
