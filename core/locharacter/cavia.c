#include "./cavia.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "core/loeffect/recipient.h"
#include "core/loentity/entity.h"
#include "core/loplayer/event.h"
#include "core/loplayer/player.h"
#include "core/loshader/character.h"

#include "./base.h"
#include "./misc.h"

static const vec2_t locharacter_cavia_size_ = vec2(.02f, .05f);

static const loeffect_recipient_status_t locharacter_cavia_base_status_ = {
  .attack  = .2f,
  .defence = .1f,
  .speed   = .05f,
};

static void
locharacter_cavia_start_walk_state_(
    locharacter_base_t* base
);
static bool
locharacter_cavia_start_thrust_state_(
    locharacter_base_t* base
);
static void
locharacter_cavia_start_cooldown_state_(
    locharacter_base_t* base
);
static void
locharacter_cavia_start_dead_state_(
    locharacter_base_t* base
);

static void locharacter_cavia_update_walk_state_(locharacter_base_t* base) {
  assert(base != NULL);

  /* ---- movement ---- */
  const vec2_t* gsize = &base->cache.ground->size;
  const float   s     = base->recipient.status.speed;
  base->pos.x += base->ticker->delta_f * base->direction * s / gsize->x;

  if (MATH_ABS(base->pos.x) > 1) base->direction *= -1;

  /* ---- motion ---- */
  loshader_character_drawer_instance_t* instance = &base->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_WALK;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;

  const int32_t p = 70/s;
  const float   t = (base->ticker->time - base->since)%p*2.0f/p - 1;
  instance->motion_time = MATH_ABS(t);

  /* ---- dead ---- */
  if (base->recipient.madness <= 0) {
    locharacter_cavia_start_dead_state_(base);
    return;
  }

  /* ---- trigger thrust ---- */
  if (loplayer_event_get_param(base->player->event) == NULL) {
    vec2_t dist;
    locommon_position_sub(
        &dist, &base->player->entity.super.super.pos, &base->super.super.pos);
    const float sdist_x = dist.x * base->direction;
    if (MATH_ABS(dist.y) < locharacter_cavia_size_.y &&
        sdist_x >= locharacter_cavia_size_.x &&
        sdist_x <= locharacter_cavia_size_.x*2) {
      if (locharacter_cavia_start_thrust_state_(base)) return;
    }
  }
}
static void locharacter_cavia_start_walk_state_(locharacter_base_t* base) {
  assert(base != NULL);

  base->since = base->ticker->time;
  base->state = LOCHARACTER_STATE_WALK;
}

static void locharacter_cavia_update_thrust_state_(locharacter_base_t* base) {
  assert(base != NULL);

  static const uint64_t premotion = 1200;
  static const uint64_t duration  = 1500;

  /* ---- motion ---- */
  loshader_character_drawer_instance_t* instance = &base->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;

  float t = (base->ticker->time - base->since)*1.0f/premotion;
  if (t > 1) t = 1;
  instance->motion_time = t*t*t*t;

  /* ---- cooldown ---- */
  if (base->since+duration <= base->ticker->time) {
    /* TODO(catfoot): go to cooldown */
    locharacter_cavia_start_cooldown_state_(base);
    return;
  }
}
static bool locharacter_cavia_start_thrust_state_(locharacter_base_t* base) {
  assert(base != NULL);

  const loplayer_combat_attack_t attack = {
    .attacker  = base->super.super.id,
    .start     = base->ticker->time + 1000,
    .duration  = 500,
    .knockback = vec2(base->direction*.1f, 0),
    .effect    = loeffect_immediate_damage(base->recipient.status.attack),
  };
  if (!loplayer_attack(base->player, &attack)) return false;

  base->since = base->ticker->time;
  base->state = LOCHARACTER_STATE_THRUST;
  return true;
}

static void locharacter_cavia_update_cooldown_state_(locharacter_base_t* base) {
  assert(base != NULL);

  static const uint64_t duration  = 500;

  /* ---- motion ---- */
  loshader_character_drawer_instance_t* instance = &base->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;

  float t = (base->ticker->time - base->since)*1.0f/duration;
  if (t > 1) t = 1;
  instance->motion_time = t*t*(3-2*t);

  /* ---- cooldown ---- */
  if (base->since+duration <= base->ticker->time) {
    if (base->recipient.madness <= 0) {
      locharacter_cavia_start_dead_state_(base);
      return;
    } else {
      locharacter_cavia_start_walk_state_(base);
      return;
    }
  }
}
static void locharacter_cavia_start_cooldown_state_(locharacter_base_t* base) {
  assert(base != NULL);

  base->since = base->ticker->time;
  base->state = LOCHARACTER_STATE_COOLDOWN;
}

static void locharacter_cavia_update_dead_state_(locharacter_base_t* base) {
  assert(base != NULL);

  static const uint64_t anime    = 500;
  static const uint64_t duration = 30000;

  /* ---- motion ---- */
  loshader_character_drawer_instance_t* instance = &base->cache.instance;
  if (base->since+anime > base->ticker->time) {
    const float t  = (base->ticker->time - base->since)*1.0f/anime;
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_SIT;
    instance->motion_time    = t*t;

  } else if (base->since+anime*2 > base->ticker->time) {
    const float t  = (base->ticker->time - anime - base->since)*1.0f/anime;
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_SIT;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_DOWN;
    instance->motion_time      = t*t;

  } else if (base->ticker->time+anime > base->since+duration) {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_DOWN;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->motion_time    =
        1 - (base->since + duration - base->ticker->time)*1.0f/anime;

  } else {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_DOWN;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_DOWN;
  }

  /* ---- revive ---- */
  if (base->since+duration <= base->ticker->time) {
    loeffect_recipient_reset(&base->recipient);
    locharacter_cavia_start_walk_state_(base);
    return;
  }
}
static void locharacter_cavia_start_dead_state_(locharacter_base_t* base) {
  assert(base != NULL);

  base->since = base->ticker->time;
  base->state = LOCHARACTER_STATE_DEAD;

  loplayer_gain_faith(base->player, .1f);
}

bool locharacter_cavia_update(locharacter_base_t* base) {
  assert(base != NULL);

  static const vec2_t size   = locharacter_cavia_size_;
  static const vec4_t color  = vec4(.1f, .1f, .1f, 1);
  static const float  height = size.y*1.4f;
  static const float  drawsz = MATH_MAX(size.x, size.y);

  loeffect_recipient_update(&base->recipient, &locharacter_cavia_base_status_);

  base->cache.instance = (loshader_character_drawer_instance_t) {
    .character_id  = LOSHADER_CHARACTER_ID_CAVIA,
    .marker_offset = vec2(0, height - drawsz),
    .pos           = vec2(0, drawsz - height),
    .size          = vec2(drawsz, drawsz),
    .color         = color,
  };

  switch (base->state) {
  case LOCHARACTER_STATE_WALK:
    locharacter_cavia_update_walk_state_(base);
    break;
  case LOCHARACTER_STATE_THRUST:
    locharacter_cavia_update_thrust_state_(base);
    break;
  case LOCHARACTER_STATE_COOLDOWN:
    locharacter_cavia_update_cooldown_state_(base);
    break;
  case LOCHARACTER_STATE_DEAD:
    locharacter_cavia_update_dead_state_(base);
    break;
  default:
    locharacter_cavia_start_walk_state_(base);
  }

  base->cache.bullet_hittest = base->state != LOCHARACTER_STATE_DEAD;

  base->cache.height = height;

  base->cache.instance.size.x *= base->direction;
  base->cache.instance.marker  = !!base->cache.bullet_hittest;
  return true;
}

void locharacter_cavia_build(
    locharacter_base_t* base, const locharacter_cavia_param_t* param) {
  assert(base  != NULL);
  assert(param != NULL);

  base->type = LOCHARACTER_TYPE_CAVIA;

  base->ground = param->ground;

  base->pos       = vec2(param->pos, 0);
  base->direction = param->direction == 1? 1: -1;

  base->state = LOCHARACTER_STATE_WALK;
  base->since = base->ticker->time;
}
