#include "./warder.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "core/lobullet/base.h"
#include "core/lobullet/linear.h"
#include "core/loeffect/recipient.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"
#include "core/loplayer/event.h"
#include "core/loplayer/player.h"
#include "core/loresource/sound.h"
#include "core/loshader/character.h"

#include "./base.h"
#include "./misc.h"

static const vec2_t locharacter_warder_size_ = vec2(.02f, .05f);

static const loeffect_recipient_status_t locharacter_warder_base_status_ = {
  .attack  = .1f,
  .defence = -.8f,
};

static void locharacter_warder_shoot_(locharacter_base_t* c) {
  assert(c != NULL);

  lobullet_base_t* b = lobullet_pool_create(c->bullets);
  lobullet_linear_light_build(b, (&(lobullet_linear_param_t) {
        .owner        = c->super.super.id,
        .pos          = c->super.super.pos,
        .size         = vec2(.04f, .04f),
        .velocity     = vec2(c->direction*.5f, 0),
        .color        = vec4(.6f, .6f, .6f, .8f),
        .acceleration = vec2(0, 0),
        .duration     = 2000,
        .knockback    = .4f,
        .effect       = loeffect_immediate_damage(c->recipient.status.attack),
      }));
  loentity_store_add(c->entities, &b->super.super);

  loresource_sound_play(c->res->sound, "enemy_shoot");
}

static void
locharacter_warder_start_wait_state_(
    locharacter_base_t* base
);
static void
locharacter_warder_start_shoot_state_(
    locharacter_base_t* base
);
static void
locharacter_warder_start_combo_state_(
    locharacter_base_t* base
);
static void
locharacter_warder_start_dead_state_(
    locharacter_base_t* base
);

static void locharacter_warder_update_wait_state_(locharacter_base_t* base) {
  assert(base != NULL);

  static const uint64_t duration = 1000;

  const uint64_t elapsed = base->ticker->time - base->since;

  /* ---- motion ---- */
  float t = elapsed*1.f / duration;
  if (t > 1) t = 1;

  loshader_character_drawer_instance_t* instance = &base->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  instance->motion_time    = t*t*(3-2*t);

  /* ---- state transition ---- */
  if (elapsed >= duration) {
    if (base->recipient.madness <= 0) {
      locharacter_warder_start_dead_state_(base);
      return;
    }

    if (loplayer_event_get_param(base->player->event) == NULL) {
      vec2_t disp;
      vec2_sub(&disp, &base->cache.player_pos, &base->pos);
      disp.x *= base->cache.ground->size.x;

      const float pdist = vec2_pow_length(&disp);
      if (MATH_ABS(disp.y) < locharacter_warder_size_.y && pdist < .4f*.4f) {
        static const float r = locharacter_warder_size_.x*3;
        if (pdist < r*r) {
          locharacter_warder_start_combo_state_(base);
        } else if (disp.x*base->direction > 0) {
          locharacter_warder_start_shoot_state_(base);
        }
      }
      return;
    }
  }
}
static void locharacter_warder_start_wait_state_(locharacter_base_t* base) {
  assert(base != NULL);

  base->since = base->ticker->time;
  base->state = LOCHARACTER_STATE_WAIT;
}

static void locharacter_warder_update_shoot_state_(locharacter_base_t* base) {
  assert(base != NULL);

  static const uint64_t duration = 500;

  const uint64_t elapsed = base->ticker->time - base->since;

  /* ---- motion ---- */
  float t = elapsed*1.f / duration;
  if (t > 1) t = 1;
  t = t*t;

  loshader_character_drawer_instance_t* instance = &base->cache.instance;
  if (t < .5f) {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->motion_time    = t*2;
  } else {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
    instance->motion_time    = (t-.5f)*2;
  }

  /* ---- state transition ---- */
  if (elapsed >= duration) {
    locharacter_warder_shoot_(base);
    locharacter_warder_start_wait_state_(base);
    return;
  }
}
static void locharacter_warder_start_shoot_state_(locharacter_base_t* base) {
  assert(base != NULL);

  base->since = base->ticker->time;
  base->state = LOCHARACTER_STATE_SHOOT;
}

static void locharacter_warder_update_combo_state_(locharacter_base_t* base) {
  assert(base != NULL);

  static const uint64_t duration = 1000;

  const uint64_t elapsed = base->ticker->time - base->since;

  /* ---- motion ---- */
  float t = elapsed*1.f/duration;
  if (t > 1) t = 1;
  t = t*t;

  loshader_character_drawer_instance_t* instance = &base->cache.instance;
  if (t < .5f) {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->motion_time    = t*2;
  } else {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
    instance->motion_time    = (t-.5f)*2;
  }

  /* ---- state transition ---- */
  if (elapsed >= duration) {
    locharacter_warder_start_wait_state_(base);
    return;
  }
}
static void locharacter_warder_start_combo_state_(locharacter_base_t* base) {
  assert(base != NULL);

  base->since = base->ticker->time;
  base->state = LOCHARACTER_STATE_COMBO;

  const float diff = base->cache.player_pos.x - base->pos.x;
  base->direction = MATH_SIGN(diff);

  const loplayer_combat_attack_t attack = {
    .attacker  = base->super.super.id,
    .start     = base->ticker->time + 500,
    .duration  = 500,
    .knockback = vec2(base->direction*.1f, 0),
    .effect    = loeffect_immediate_damage(base->recipient.status.attack),
  };
  loplayer_attack(base->player, &attack);
}

static void locharacter_warder_update_dead_state_(locharacter_base_t* base) {
  assert(base != NULL);

  static const uint64_t anime_duration = 500;
  static const uint64_t duration       = 30000;

  const uint64_t elapsed = base->ticker->time - base->since;

  /* ---- motion ---- */
  loshader_character_drawer_instance_t* instance = &base->cache.instance;
  if (elapsed > duration - anime_duration) {  /* wake up */
    float t = 1-(duration - elapsed)*1.f/anime_duration;
    if (t < 0) t = 0;
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_DOWN;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->motion_time    = 1-powf(1-t, 2);
  } else {  /* down */
    float t = elapsed*1.f/anime_duration;
    if (t > 1) t = 1;
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_DOWN;
    instance->motion_time    = t;
  }

  /* ---- state transition ---- */
  if (elapsed >= duration) {
    loeffect_recipient_reset(&base->recipient);
    locharacter_warder_start_wait_state_(base);
    return;
  }
}
static void locharacter_warder_start_dead_state_(locharacter_base_t* base) {
  assert(base != NULL);

  base->since = base->ticker->time;
  base->state = LOCHARACTER_STATE_DEAD;

  loplayer_gain_faith(base->player, .1f);
}

bool locharacter_warder_update(locharacter_base_t* base) {
  assert(base != NULL);

  static const vec2_t size   = locharacter_warder_size_;
  static const vec4_t color  = vec4(.1f, .1f, .1f, 1);
  static const float  height = size.y*1.4f;
  static const float  drawsz = MATH_MAX(size.x, size.y);

  loeffect_recipient_update(&base->recipient, &locharacter_warder_base_status_);

  base->cache.instance = (loshader_character_drawer_instance_t) {
    .character_id  = LOSHADER_CHARACTER_ID_WARDER,
    .marker_offset = vec2(0, height - drawsz),
    .pos           = vec2(0, drawsz - height),
    .size          = vec2(drawsz, drawsz),
    .color         = color,
  };

  switch (base->state) {
  case LOCHARACTER_STATE_WAIT:
    locharacter_warder_update_wait_state_(base);
    break;
  case LOCHARACTER_STATE_SHOOT:
    locharacter_warder_update_shoot_state_(base);
    break;
  case LOCHARACTER_STATE_COMBO:
    locharacter_warder_update_combo_state_(base);
    break;
  case LOCHARACTER_STATE_DEAD:
    locharacter_warder_update_dead_state_(base);
    break;
  default:
    locharacter_warder_start_wait_state_(base);
  }

  base->cache.bullet_hittest = base->state != LOCHARACTER_STATE_DEAD;
  base->cache.gravity        = true;

  base->cache.height = height;

  base->cache.instance.size.x *= base->direction;
  base->cache.instance.marker  = !!base->cache.bullet_hittest;
  return true;
}

void locharacter_warder_build(
    locharacter_base_t* base, const locharacter_warder_param_t* param) {
  assert(base  != NULL);
  assert(param != NULL);

  base->type = LOCHARACTER_TYPE_WARDER;

  base->ground = param->ground;

  base->pos       = vec2(param->pos, 0);
  base->direction = -MATH_SIGN(param->pos);
  if (base->direction == 0) base->direction = 1;

  locharacter_warder_start_wait_state_(base);
}
