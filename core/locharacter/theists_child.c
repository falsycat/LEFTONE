#include "./theists_child.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <msgpack.h>

#include "util/chaos/xorshift.h"
#include "util/math/algorithm.h"
#include "util/math/constant.h"
#include "util/math/rational.h"
#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/lobullet/base.h"
#include "core/lobullet/bomb.h"
#include "core/lobullet/linear.h"
#include "core/lobullet/pool.h"
#include "core/locommon/msgpack.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/recipient.h"
#include "core/loeffect/stance.h"
#include "core/loentity/bullet.h"
#include "core/loentity/store.h"
#include "core/loplayer/event.h"
#include "core/loplayer/player.h"
#include "core/loresource/music.h"
#include "core/loresource/set.h"
#include "core/loresource/text.h"
#include "core/loshader/character.h"

#include "./base.h"
#include "./misc.h"
#include "./util.h"

typedef struct {
  locharacter_event_holder_t event;

  uint64_t phase;
  vec2_t   from;
  vec2_t   to;
} locharacter_theists_child_param_t;

#define LOCHARACTER_THEISTS_CHILD_PARAM_TO_PACK_EACH_(PROC) do {  \
  PROC("event-start-time", event.start_time);  \
  PROC("phase",            phase);  \
  PROC("from",             from);  \
  PROC("to",               to);  \
} while (0)
#define LOCHARACTER_THEISTS_CHILD_PARAM_TO_PACK_COUNT 4

static const vec2_t locharacter_theists_child_size_ = vec2(.02f, .06f);

static const loeffect_recipient_status_t
locharacter_theists_child_base_status_ = {
  .attack  = .1f,
  .defence = .9f,
  .speed   = .1f,
  .jump    = .2f,
};

#define LOCHARACTER_THEISTS_CHILD_BEAT (60000/140.f)  /* 140 BPM */
#define LOCHARACTER_THEISTS_CHILD_MUSIC_DURATION  \
    ((uint64_t) LOCHARACTER_THEISTS_CHILD_BEAT*236)

#define LOCHARACTER_THEISTS_CHILD_MELODY_B_START_BEAT 128
#define LOCHARACTER_THEISTS_CHILD_MELODY_B_END_BEAT   192

#include "./theists_child.private.h"

static void
locharacter_theists_child_start_wait_state_(
    locharacter_base_t* c
);
static void
locharacter_theists_child_start_standup_state_(
    locharacter_base_t* c
);
static void
locharacter_theists_child_start_rush_state_(
    locharacter_base_t* c
);
static void
locharacter_theists_child_start_combo_state_(
    locharacter_base_t* c
);
static void
locharacter_theists_child_start_cooldown_state_(
    locharacter_base_t* c
);
static void
locharacter_theists_child_start_stunned_state_(
    locharacter_base_t* c
);
static void
locharacter_theists_child_start_dead_state_(
    locharacter_base_t* c
);

static void locharacter_theists_child_finalize_event_(locharacter_base_t* c) {
  assert(c != NULL);
  /* This function must start next state. */

  locharacter_theists_child_param_t* p = (typeof(p)) c->data;
  assert(p != NULL);

  locharacter_event_holder_release_control(&p->event);

  if (c->recipient.madness > 0) {
    loentity_character_apply_effect(
        &c->player->entity.super, &loeffect_curse_trigger());
    locharacter_theists_child_start_wait_state_(c);
  } else {
    loplayer_gain_stance(c->player, LOEFFECT_STANCE_ID_REVOLUTIONER);
    locharacter_theists_child_start_dead_state_(c);
  }
}

static bool locharacter_theists_child_reset_if_player_left_(
    locharacter_base_t* c) {
  assert(c != NULL);

  locharacter_theists_child_param_t* p = (typeof(p)) c->data;

  if (MATH_ABS(c->cache.player_pos.x) < 1 &&
      0 < c->cache.player_pos.y && c->cache.player_pos.y < 1) {
    return false;
  }
  locharacter_event_holder_release_control(&p->event);
  locharacter_theists_child_start_wait_state_(c);
  return true;
}

static void locharacter_theists_child_fire_bullets_(locharacter_base_t* c) {
  assert(c != NULL);

  static const float    len   = .3f;
  static const float    accel = .6f;
  static const uint64_t dur   = 1000;

  for (size_t i = 0; i < 30; ++i) {
    const float t = MATH_PI/15*i;

    const vec2_t v = vec2(cos(t), sin(t));

    locommon_position_t pos = c->super.super.pos;
    pos.fract.x += v.x*len;
    pos.fract.y += v.y*len;
    locommon_position_reduce(&pos);

    lobullet_base_t* bullet = lobullet_pool_create(c->bullets);
    lobullet_linear_light_build(bullet, (&(lobullet_linear_param_t) {
        .owner        = c->super.super.id,
        .pos          = pos,
        .size         = vec2(.015f, .015f),
        .acceleration = vec2(-v.x*accel, -v.y*accel),
        .color        = vec4(1, 1, 1, .8f),
        .duration     = dur,
        .knockback    = .1f,
        .effect       = loeffect_immediate_damage(c->recipient.status.attack),
    }));
    loentity_store_add(c->entities, &bullet->super.super);
  }
}

static void locharacter_theists_child_update_wait_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float   standup_range = .5f;
  static const int32_t sit_duration  = 4000;

  c->cache.gravity = true;

  /* ---- motion ---- */
  float t = (c->cache.time - c->since)*1.f/sit_duration;
  if (t > 1) t = 1;
  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_DOWN;
  instance->motion_time    = t;

  /* ---- state transition ---- */
  if (c->since+sit_duration <= c->cache.time) {
    if (MATH_ABS(c->cache.player_pos.x) < 1 &&
        0 < c->cache.player_pos.y && c->cache.player_pos.y < 1) {
      vec2_t diff;
      vec2_sub(&diff, &c->cache.player_pos, &c->pos);
      if (vec2_pow_length(&diff) < standup_range*standup_range) {
        locharacter_theists_child_start_standup_state_(c);
        return;
      }
    }
  }
}
static void locharacter_theists_child_start_wait_state_(locharacter_base_t* c) {
  assert(c != NULL);

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_WAIT;
}

static void locharacter_theists_child_update_standup_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat          = LOCHARACTER_THEISTS_CHILD_BEAT;
  static const uint64_t line_duration = beat*10;

  locharacter_theists_child_param_t* p = (typeof(p)) c->data;

  const bool event = locharacter_event_holder_has_control(&p->event);
  const uint64_t standup_duration = event? beat*64: 1000;

  /* ---- motion ---- */
  float t = (c->cache.time - c->since)*1.0f/standup_duration;
  if (t > 1) t = 1;

  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  if (t < .5f) {
    t *= 2;
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_DOWN;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_SIT;
    instance->motion_time    = t*t*(3-2*t);
  } else {
    t = (t-.5f)*2;
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_SIT;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->motion_time    = t*t*(3-2*t);
  }

  /* ---- event ---- */
  if (event) {
    p->event.param->cinescope = true;
    p->event.param->hide_hud  = true;

    if (c->since+(p->phase+2)*line_duration < c->cache.time) {
      static const char* text[] = {
        "boss_theists_child_line0",
        "boss_theists_child_line1",
        "boss_theists_child_line2",
      };
      if (p->phase < sizeof(text)/sizeof(text[0])) {
        const char* v = loresource_text_get(
            c->res->lang, text[(size_t) p->phase]);
        loplayer_event_param_set_line(p->event.param, v, strlen(v));
      } else {
        loplayer_event_param_set_line(p->event.param, "", 0);
      }
      ++p->phase;
    }
  }

  /* ---- state transition ---- */
  if (locharacter_theists_child_reset_if_player_left_(c)) return;

  if (c->since+standup_duration < c->cache.time) {
    if (event) {
      p->event.param->hide_hud  = false;
      p->event.param->cinescope = false;
      loplayer_event_param_set_line(p->event.param, "", 0);
    }
    locharacter_theists_child_start_rush_state_(c);
    return;
  }
}
static void locharacter_theists_child_start_standup_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  locharacter_theists_child_param_t* p = (typeof(p)) c->data;

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_STANDUP;

  p->phase = 0;
  loeffect_recipient_reset(&c->recipient);

  if (!loeffect_stance_set_has(
        &c->player->status.stances, LOEFFECT_STANCE_ID_REVOLUTIONER)) {
    locharacter_event_holder_take_control(&p->event);
  }
}

static void locharacter_theists_child_update_rush_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float beat = LOCHARACTER_THEISTS_CHILD_BEAT;

  static const uint64_t premotion_duration = beat*2;
  static const uint64_t whole_duration     = beat*4;

  const locharacter_theists_child_param_t* p = (typeof(p)) c->data;

  const uint64_t elapsed = c->cache.time - c->since;

  /* ---- motion ---- */
  float t = elapsed*1.f/premotion_duration;
  if (t > 1) t = 1;

  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  if (t < .1f) {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_WALK;
    instance->motion_time    = 1-powf(1-t*10, 2);
  } else if (t < .5f) {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_WALK;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->motion_time    = 1-powf(1-(t-.1f)/4*10, 2);
  } else {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
    instance->motion_time    = powf((t-.5f)*2, 4);
  }

  /* ---- position ---- */
  vec2_sub(&c->pos, &p->to, &p->from);
  c->direction = MATH_SIGN(c->pos.x);
  vec2_muleq(&c->pos, powf(t, 2));
  c->pos.y += (1-MATH_ABS(t*2-1))*c->recipient.status.jump*.1f;
  vec2_addeq(&c->pos, &p->from);

  /* ---- state transition ---- */
  if (locharacter_theists_child_reset_if_player_left_(c)) return;

  if (c->since+whole_duration < c->cache.time) {
    locharacter_theists_child_start_cooldown_state_(c);
    return;
  }
}
static void locharacter_theists_child_start_rush_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float beat = LOCHARACTER_THEISTS_CHILD_BEAT;

  static const uint64_t parry = 300;

  locharacter_theists_child_param_t* p = (typeof(p)) c->data;

  if (c->last_knockback_time + parry > c->cache.time) {
    locharacter_theists_child_start_stunned_state_(c);
    return;
  }

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_RUSH;

  const vec2_t* player = &c->cache.player_pos;

  const float diffx = player->x - c->pos.x;
  p->from = c->pos;
  p->to   = vec2(
        player->x - MATH_SIGN(diffx)*locharacter_theists_child_size_.x*2,
        player->y - .02f);

  const loplayer_combat_attack_t attack = {
    .attacker  = c->super.super.id,
    .start     = c->ticker->time + (uint64_t) beat,
    .duration  = beat*3,
    .knockback = vec2(MATH_SIGN(player->x)*.2f, 0),
    .effect    = loeffect_immediate_damage(c->recipient.status.attack*2),
  };
  loplayer_attack(c->player, &attack);
}

static void locharacter_theists_child_update_combo_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat               = LOCHARACTER_THEISTS_CHILD_BEAT;
  static const uint64_t premotion_duration = beat;
  static const uint64_t attack_duration    = beat;
  static const uint64_t whole_duration     = beat*4;

  const locharacter_theists_child_param_t* p = (typeof(p)) c->data;

  const uint64_t elapsed = c->cache.time - c->since;

  /* ---- motion ---- */
  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  if (elapsed < premotion_duration) {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->motion_time    = elapsed*1.f / premotion_duration;
  } else {
    const uint64_t attack_elapsed = elapsed - premotion_duration;

    float t = 1;
    if (attack_elapsed < attack_duration*p->phase) {
      t = attack_elapsed%attack_duration*1.f / attack_duration;
    }
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
    instance->motion_time    = 1-powf(1-t, 4);
  }

  /* ---- position ---- */
  if (elapsed < premotion_duration) {
    const float t = elapsed*1.f/premotion_duration;
    vec2_sub(&c->pos, &p->to, &p->from);
    vec2_muleq(&c->pos, t*t);
    vec2_addeq(&c->pos, &p->from);
  }

  /* ---- state transition ---- */
  if (locharacter_theists_child_reset_if_player_left_(c)) return;

  if (elapsed >= whole_duration) {
    locharacter_theists_child_start_cooldown_state_(c);
    return;
  }
}
static void locharacter_theists_child_start_combo_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat  = LOCHARACTER_THEISTS_CHILD_BEAT;
  static const uint64_t parry = 200;

  locharacter_theists_child_param_t* p = (typeof(p)) c->data;

  if (c->last_knockback_time + parry > c->cache.time) {
    locharacter_theists_child_start_stunned_state_(c);
    return;
  }

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_COMBO;

  const float diffx = c->cache.player_pos.x - c->pos.x;
  c->direction  = MATH_SIGN(diffx);

  p->phase      = 2 + chaos_xorshift(c->since)%2;
  p->from  = c->pos;
  p->to    = c->cache.player_pos;
  p->to.x -= c->direction*locharacter_theists_child_size_.x*2;
  p->to.y -= .02f;

  const loplayer_combat_attack_t attack1 = {
    .attacker  = c->super.super.id,
    .start     = c->ticker->time + (uint64_t) beat,
    .duration  = beat/2,
    .knockback = vec2(c->direction*.1f, 0),
    .effect    = loeffect_immediate_damage(c->recipient.status.attack*.8f),
  };
  loplayer_attack(c->player, &attack1);

  const loplayer_combat_attack_t attack2 = {
    .attacker = c->super.super.id,
    .start    = c->ticker->time + (uint64_t) (beat*2),
    .duration = p->phase == 2? beat*1.5: beat/2,
    .knockback = vec2(c->direction*.1f, 0),
    .effect    = loeffect_immediate_damage(c->recipient.status.attack*1.1f),
  };
  loplayer_attack(c->player, &attack2);

  if (p->phase >= 3) {
    const loplayer_combat_attack_t attack3 = {
      .attacker = c->super.super.id,
      .start    = c->ticker->time + (uint64_t) (beat*3),
      .duration = beat/2,
      .knockback = vec2(c->direction*.1f, 0),
      .effect    = loeffect_immediate_damage(c->recipient.status.attack*1.3f),
    };
    loplayer_attack(c->player, &attack3);
  }
}

static void locharacter_theists_child_update_cooldown_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float beat = LOCHARACTER_THEISTS_CHILD_BEAT;

  static const uint64_t duration = beat*4;

  const locharacter_theists_child_param_t* p = (typeof(p)) c->data;

  c->cache.bullet_hittest = true;
  c->cache.gravity        = true;

  /* ---- motion ---- */
  float t = (c->cache.time - c->since)*1.0f/duration;
  if (t > 1) t = 1;

  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  instance->motion_time    = t;

  /* ---- state transition ---- */
  if (locharacter_theists_child_reset_if_player_left_(c)) return;

  if (c->since+duration < c->cache.time) {
    if (locharacter_event_holder_has_control(&p->event)) {
      static const uint64_t dur = LOCHARACTER_THEISTS_CHILD_MUSIC_DURATION;
      if (p->event.start_time+dur < c->cache.time) {
        locharacter_theists_child_finalize_event_(c);
        return;
      }
    } else {
      if (c->recipient.madness <= 0) {
        locharacter_theists_child_start_dead_state_(c);
        return;
      }
    }

    vec2_t diff;
    vec2_sub(&diff, &c->cache.player_pos, &c->pos);
    if (vec2_pow_length(&diff) < .5f*.5f) {
      locharacter_theists_child_start_combo_state_(c);
      return;
    }

    locharacter_theists_child_start_rush_state_(c);
    return;
  }
}
static void locharacter_theists_child_start_cooldown_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  const locharacter_theists_child_param_t* p = (typeof(p)) c->data;

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_COOLDOWN;

  bool skip_firing = false;
  if (locharacter_event_holder_has_control(&p->event)) {
    const float beat =
        (c->cache.time - p->event.start_time)/LOCHARACTER_THEISTS_CHILD_BEAT;
    skip_firing =
        LOCHARACTER_THEISTS_CHILD_MELODY_B_START_BEAT <= beat &&
        beat < LOCHARACTER_THEISTS_CHILD_MELODY_B_END_BEAT;
  }
  if (!skip_firing) locharacter_theists_child_fire_bullets_(c);
}

static void locharacter_theists_child_update_stunned_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat     = LOCHARACTER_THEISTS_CHILD_BEAT;
  static const uint64_t duration = beat*4;

  /* ---- motion ---- */
  float t = (c->cache.time - c->since)*1.f/duration;
  t *= 6;

  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  if (t < 1) {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_SIT;
    instance->motion_time    = 1-powf(1-t, 6);
  } else {
    t = (t-1)/5;
    if (t > 1) t = 1;
    t = t*t*(3-2*t);

    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_SIT;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
    instance->motion_time    = t;
  }

  /* ---- state transition ---- */
  if (c->since+duration < c->cache.time) {
    locharacter_theists_child_start_cooldown_state_(c);
    return;
  }
}
static void locharacter_theists_child_start_stunned_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_STUNNED;

  loeffect_recipient_apply_effect(
      &c->recipient, &loeffect_immediate_damage(1.f));
}

static void locharacter_theists_child_update_dead_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const uint64_t anime_duration = 4000;
  static const uint64_t duration       = 30000;

  c->cache.gravity = true;

  /* ---- motion ---- */
  float t = (c->cache.time - c->since)*1.f/anime_duration;
  if (t > 1) t = 1;

  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_DOWN;
  instance->motion_time    = t*t;
  instance->color.w        = 1-t;

  /* ---- state transition ---- */
  if (c->since+duration < c->cache.time) {
    c->pos = vec2(0, 0);
    locharacter_theists_child_start_wait_state_(c);
    return;
  }
}
static void locharacter_theists_child_start_dead_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_DEAD;

  loplayer_gain_faith(c->player, .5f);
}

bool locharacter_theists_child_update(locharacter_base_t* base) {
  assert(base != NULL);

  static const vec2_t size   = locharacter_theists_child_size_;
  static const vec4_t color  = vec4(.05f, 0, 0, 1);
  static const float  height = size.y * 1.4f;
  static const float  drawsz = MATH_MAX(size.x, size.y);

  locharacter_theists_child_param_t* p = (typeof(p)) base->data;

  loeffect_recipient_update(
      &base->recipient, &locharacter_theists_child_base_status_);

  if (!locharacter_event_holder_update(&p->event)) {
    locharacter_theists_child_start_wait_state_(base);
  }

  base->cache.instance = (loshader_character_drawer_instance_t) {
    .character_id  = LOSHADER_CHARACTER_ID_CAVIA,
    .marker_offset = vec2(0, height - drawsz),
    .pos           = vec2(0, drawsz - height),
    .size          = vec2(drawsz, drawsz),
    .color         = color,
  };

  switch (base->state) {
  case LOCHARACTER_STATE_WAIT:
    locharacter_theists_child_update_wait_state_(base);
    break;
  case LOCHARACTER_STATE_STANDUP:
    locharacter_theists_child_update_standup_state_(base);
    break;
  case LOCHARACTER_STATE_RUSH:
    locharacter_theists_child_update_rush_state_(base);
    break;
  case LOCHARACTER_STATE_COMBO:
    locharacter_theists_child_update_combo_state_(base);
    break;
  case LOCHARACTER_STATE_COOLDOWN:
    locharacter_theists_child_update_cooldown_state_(base);
    break;
  case LOCHARACTER_STATE_STUNNED:
    locharacter_theists_child_update_stunned_state_(base);
    break;
  case LOCHARACTER_STATE_DEAD:
    locharacter_theists_child_update_dead_state_(base);
    break;
  default:
    locharacter_theists_child_start_wait_state_(base);
  }
  locharacter_theists_child_update_passive_action_(base);

  base->cache.height = height;

  base->cache.instance.marker  = !!base->cache.bullet_hittest;
  base->cache.instance.size.x *= base->direction;
  return true;
}

void locharacter_theists_child_build(locharacter_base_t* base, loentity_id_t ground) {
  assert(base != NULL);

  base->type = LOCHARACTER_TYPE_THEISTS_CHILD;

  base->ground = ground;

  base->pos       = vec2(0, 0);
  base->direction = 1;

  base->state = LOCHARACTER_STATE_WAIT;
  base->since = base->cache.time;

  locharacter_theists_child_param_t* p = (typeof(p)) base->data;
  *p = (typeof(*p)) {0};

  locharacter_event_holder_initialize(
      &p->event,
      &base->res->music.boss_theists_child,
      base,
      LOCHARACTER_THEISTS_CHILD_MUSIC_DURATION,
      0);
}

void locharacter_theists_child_tear_down(locharacter_base_t* base) {
  assert(base != NULL);

  locharacter_theists_child_param_t* p = (typeof(p)) base->data;
  locharacter_event_holder_deinitialize(&p->event);
}

void locharacter_theists_child_pack_data(
    const locharacter_base_t* base, msgpack_packer* packer) {
  assert(base   != NULL);
  assert(packer != NULL);

  const locharacter_theists_child_param_t* p = (typeof(p)) base->data;

  msgpack_pack_map(packer, LOCHARACTER_THEISTS_CHILD_PARAM_TO_PACK_COUNT);

# define pack_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &p->var);  \
  } while (0)

  LOCHARACTER_THEISTS_CHILD_PARAM_TO_PACK_EACH_(pack_);

# undef pack_
}

bool locharacter_theists_child_unpack_data(
    locharacter_base_t* base, const msgpack_object* obj) {
  assert(base != NULL);

  locharacter_theists_child_param_t* p = (typeof(p)) base->data;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define unpack_(name, var) do {  \
    if (!LOCOMMON_MSGPACK_UNPACK_ANY(item_(name), &p->var)) {  \
      return false;  \
    }  \
  } while (0)

  LOCHARACTER_THEISTS_CHILD_PARAM_TO_PACK_EACH_(unpack_);

# undef unpack_

# undef item_

  locharacter_event_holder_initialize(
      &p->event,
      &base->res->music.boss_theists_child,
      base,
      LOCHARACTER_THEISTS_CHILD_MUSIC_DURATION,
      p->event.start_time);
  return true;
}
