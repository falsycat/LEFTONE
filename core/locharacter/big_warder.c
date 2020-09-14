#include "./big_warder.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/lobullet/base.h"
#include "core/lobullet/bomb.h"
#include "core/lobullet/linear.h"
#include "core/lobullet/pool.h"
#include "core/locommon/easing.h"
#include "core/locommon/msgpack.h"
#include "core/loentity/entity.h"
#include "core/loplayer/combat.h"
#include "core/loplayer/event.h"
#include "core/loplayer/player.h"
#include "core/loresource/music.h"
#include "core/loresource/text.h"
#include "core/loshader/character.h"

#include "./base.h"
#include "./misc.h"
#include "./util.h"

typedef struct {
  locharacter_event_holder_t event;

  int32_t phase;
  vec2_t  from;
  vec2_t  to;
} locharacter_big_warder_param_t;

_Static_assert(
    sizeof(locharacter_big_warder_param_t) <= LOCHARACTER_BASE_DATA_MAX_SIZE);

#define LOCHARACTER_BIG_WARDER_PARAM_TO_PACK_EACH_(PROC) do {  \
  PROC("event-start-time", event.start_time);  \
  PROC("phase",            phase);  \
  PROC("from",             from);  \
  PROC("to",               to);  \
} while (0)
#define LOCHARACTER_BIG_WARDER_PARAM_TO_PACK_COUNT 4

static const vec2_t locharacter_big_warder_size_ = vec2(.04f, .07f);

static const loeffect_recipient_status_t
locharacter_big_warder_base_status_ = {
  .attack  = .1f,
  .defence = .85f,
  .speed   = .1f,
  .jump    = .1f,
};

#define LOCHARACTER_BIG_WARDER_BEAT (60000/80.f)  /* 80 BPM */
#define LOCHARACTER_BIG_WARDER_MUSIC_DURATION  \
    ((uint64_t) LOCHARACTER_BIG_WARDER_BEAT*144)

#define LOCHARACTER_BIG_WARDER_MELODY_B_BEAT 80

#include "./big_warder.private.h"

static void
locharacter_big_warder_start_wait_state_(
    locharacter_base_t* c
);
static void
locharacter_big_warder_start_walk_state_(
    locharacter_base_t* c
);
static void
locharacter_big_warder_start_shoot_state_(
    locharacter_base_t* c
);
static void
locharacter_big_warder_start_combo_state_(
    locharacter_base_t* c
);
static void
locharacter_big_warder_start_thrust_state_(
    locharacter_base_t* c
);
static void
locharacter_big_warder_start_cooldown_state_(
    locharacter_base_t* c
);
static void
locharacter_big_warder_start_stunned_state_(
    locharacter_base_t* c
);
static void
locharacter_big_warder_start_dead_state_(
    locharacter_base_t* c
);

static void locharacter_big_warder_finalize_event_(locharacter_base_t* c) {
  assert(c != NULL);
  /* This function must start next state. */

  locharacter_big_warder_param_t* p = (typeof(p)) c->data;
  assert(p != NULL);

  locharacter_event_holder_release_control(&p->event);

  if (c->recipient.madness > 0) {
    loentity_character_apply_effect(
        &c->player->entity.super, &loeffect_curse_trigger());
    locharacter_big_warder_start_wait_state_(c);
  } else {
    loplayer_gain_stance(c->player, LOEFFECT_STANCE_ID_UNFINISHER);
    locharacter_big_warder_start_dead_state_(c);
  }
}

static bool locharacter_big_warder_reset_if_player_left_(
    locharacter_base_t* c) {
  assert(c != NULL);

  if (MATH_ABS(c->cache.player_pos.x) < 1 &&
      0 < c->cache.player_pos.y && c->cache.player_pos.y < 1) {
    return false;
  }

  locharacter_big_warder_param_t* p = (typeof(p)) c->data;
  locharacter_event_holder_release_control(&p->event);

  locharacter_big_warder_start_wait_state_(c);
  return true;
}

static void locharacter_big_warder_update_wait_state_(locharacter_base_t* c) {
  assert(c != NULL);

  static const uint64_t period = 1000;

  c->cache.gravity = true;

  /* ---- motion ---- */
  float t = (c->cache.time - c->since)%period*1.f/period;
  t = (t*2) - 1;
  t = MATH_ABS(t);

  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  instance->motion_time    = t*t*(3-2*t);

  /* ---- state transition ---- */
  if (MATH_ABS(c->cache.player_pos.x) < 1 &&
      0 < c->cache.player_pos.y && c->cache.player_pos.y < 1) {
    vec2_t diff;
    vec2_sub(&diff, &c->cache.player_pos, &c->pos);
    if (vec2_pow_length(&diff) < .5f*.5f) {
      locharacter_big_warder_start_walk_state_(c);
      return;
    }
  }
}
static void locharacter_big_warder_start_wait_state_(locharacter_base_t* c) {
  assert(c != NULL);

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_WAIT;
}

static void locharacter_big_warder_update_walk_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat    = LOCHARACTER_BIG_WARDER_BEAT;
  static const float    linedur = beat*4;
  static const uint64_t period  = 800;

  locharacter_big_warder_param_t* p = (typeof(p)) c->data;

  const bool event = locharacter_event_holder_has_control(&p->event);

  const uint64_t min_duration = event? LOCHARACTER_BIG_WARDER_BEAT*16: 0;

  /* ---- motion ---- */
  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  if (c->pos.x != 0) {
    float t = (c->cache.time - c->since)%period*1.f/period;
    t = (t*2) - 1;
    t = MATH_ABS(t);
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_WALK;
    instance->motion_time    = t;
  } else {
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  }

  /* ---- position ---- */
  if (c->pos.x != 0) c->direction = -MATH_SIGN(c->pos.x);

  c->pos.y = 0;
  locommon_easing_linear_float(&c->pos.x, 0, c->ticker->delta_f/5);

  /* ---- event ---- */
  if (event) {
    p->event.param->cinescope = true;
    p->event.param->hide_hud  = true;

    if (c->since+(p->phase+1)*linedur < c->cache.time) {
      static const char* text[] = {
        "boss_big_warder_line0",
        "boss_big_warder_line1",
      };
      if (p->phase < (int32_t) (sizeof(text)/sizeof(text[0]))) {
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
  if (locharacter_big_warder_reset_if_player_left_(c)) return;

  if (c->pos.x == 0 && c->since + min_duration <= c->cache.time) {
    if (event) {
      p->event.param->hide_hud  = false;
      p->event.param->cinescope = false;
      loplayer_event_param_set_line(p->event.param, "", 0);
    }
    locharacter_big_warder_start_shoot_state_(c);
    return;
  }
}
static void locharacter_big_warder_start_walk_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  locharacter_big_warder_param_t* p = (typeof(p)) c->data;

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_WALK;

  p->phase = 0;

  loeffect_recipient_reset(&c->recipient);

  if (!loeffect_stance_set_has(
        &c->player->status.stances, LOEFFECT_STANCE_ID_UNFINISHER)) {
    locharacter_event_holder_take_control(&p->event);
  }
}

static void locharacter_big_warder_update_shoot_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat     = LOCHARACTER_BIG_WARDER_BEAT;
  static const uint64_t duration = beat*3;

  const uint64_t t  = c->cache.time - c->since;

  c->cache.bullet_hittest = true;

  /* ---- motion ---- */
  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  instance->motion_time    = t;

  /* ---- shooting ---- */
  locharacter_big_warder_param_t* p = (typeof(p)) c->data;
  if (p->phase < 4 && p->phase*beat/2 <= c->cache.time - c->since) {
    ++p->phase;
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
          .effect       = loeffect_immediate_damage(
              c->recipient.status.attack/2),
        }));
    loentity_store_add(c->entities, &b->super.super);
  }

  /* ---- state transition ---- */
  if (locharacter_big_warder_reset_if_player_left_(c)) return;

  if (c->since + duration <= c->cache.time) {
    locharacter_big_warder_start_combo_state_(c);
    return;
  }
}
static void locharacter_big_warder_start_shoot_state_(locharacter_base_t* c) {
  assert(c != NULL);

  locharacter_big_warder_param_t* p = (typeof(p)) c->data;

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_SHOOT;

  c->direction = c->cache.player_pos.x - c->pos.x;
  c->direction = c->direction > 0? 1: -1;

  p->phase = 0;
}

static void locharacter_big_warder_update_combo_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat       = LOCHARACTER_BIG_WARDER_BEAT;
  static const uint64_t step_dur   = beat;
  static const uint64_t attack_dur = beat*2;

  const locharacter_big_warder_param_t* p = (typeof(p)) c->data;

  c->cache.bullet_hittest = true;

  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  if (c->since + step_dur > c->cache.time) {
    const float t = (c->cache.time - c->since)*1.f/step_dur;

    /* ---- position ---- */
    vec2_t dist;
    vec2_sub(&dist, &p->to, &p->from);
    vec2_muleq(&dist, t*t*(3-2*t));
    c->pos = p->from;
    vec2_addeq(&c->pos, &dist);

    /* ---- motion ---- */
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_WALK;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->motion_time    = 1-powf(2*t-1, 4);
  } else {
    float t = (c->cache.time - c->since - step_dur)*1.f/attack_dur;
    t *= 4;
    t -= (uint64_t) t;

    /* ---- motion ---- */
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
    instance->motion_time    = t*t;
  }

  /* ---- state transition ---- */
  if (locharacter_big_warder_reset_if_player_left_(c)) return;

  if (c->since + step_dur + attack_dur <= c->cache.time) {
    locharacter_big_warder_start_thrust_state_(c);
    return;
  }
}
static void locharacter_big_warder_start_combo_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat  = LOCHARACTER_BIG_WARDER_BEAT;
  static const uint64_t parry = 400;

  locharacter_big_warder_param_t* p = (typeof(p)) c->data;

  if (c->last_knockback_time+parry > c->cache.time) {
    locharacter_big_warder_start_stunned_state_(c);
    return;
  }

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_COMBO;

  c->direction = c->cache.player_pos.x - c->pos.x;
  c->direction = c->direction > 0? 1: -1;

  c->gravity = 0;

  p->from  = c->pos;
  p->to    = c->cache.player_pos;
  p->to.x -= c->direction * locharacter_big_warder_size_.x;
  p->to.y -= locharacter_big_warder_size_.y*.2f;

  for (size_t i = 0; i < 4; ++i) {
    loplayer_attack(c->player, &(loplayer_combat_attack_t) {
      .attacker  = c->super.super.id,
      .start     = c->ticker->time + (uint64_t) (beat*(i/2.f+1)),
      .duration  = beat/4,
      .knockback = vec2(c->direction*.1f, 0),
      .effect    = loeffect_immediate_damage(c->recipient.status.attack),
    });
  }
}

static void locharacter_big_warder_update_thrust_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat     = LOCHARACTER_BIG_WARDER_BEAT;
  static const uint64_t duration = beat;

  locharacter_big_warder_param_t* p = (typeof(p)) c->data;

  c->cache.bullet_hittest = true;

  /* ---- motion ---- */
  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;

  const uint64_t ti = c->cache.time - c->since;
  if (p->phase <= 0) {
    /* ---- disappear ---- */
    instance->color.w *= 1 - ti/beat/4;
    if (ti > beat/4) {
      c->pos = p->to;
      if (p->phase < 0) {  /* backattack */
        c->direction *= -1;
        p->phase = 0;
      }
      ++p->phase;
    }

  } else if (p->phase == 1) {
    /* ---- appear ---- */
    float t = (ti/beat/2 - .5f)*2;
    if (t > 1) t = 1;

    instance->to_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->motion_time  = t;
    instance->color.w     *= t;

    if (ti > beat/2) ++p->phase;

  } else {
    /* ---- attack ---- */
    float t = (ti/beat - .5f)*2;
    if (t > 1) t = 1;

    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
    instance->motion_time    = t;
  }

  /* ---- state transition ---- */
  if (locharacter_big_warder_reset_if_player_left_(c)) return;

  if (c->since + duration < c->cache.time) {
    locharacter_big_warder_start_cooldown_state_(c);
    return;
  }
}
static void locharacter_big_warder_start_thrust_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float beat  = LOCHARACTER_BIG_WARDER_BEAT;
  static const float bmelo = LOCHARACTER_BIG_WARDER_MELODY_B_BEAT;

  locharacter_big_warder_param_t* p = (typeof(p)) c->data;

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_THRUST;

  c->direction = c->cache.player_pos.x - c->pos.x;
  c->direction = c->direction > 0? 1: -1;

  c->gravity = 0;

  const bool backattack =
      locharacter_event_holder_has_control(&p->event) &&
      (c->cache.time - p->event.start_time >= beat*bmelo);
  const float backattack_f = backattack? 1: -1;

  p->to    = c->cache.player_pos;
  p->to.x += c->direction*locharacter_big_warder_size_.x*backattack_f;
  p->to.y -= locharacter_big_warder_size_.y*.2f;

  p->phase = backattack? -1: 0;

  loplayer_attack(c->player, &(loplayer_combat_attack_t) {
    .attacker  = c->super.super.id,
    .start     = c->ticker->time + (uint64_t) (beat/2),
    .duration  = beat/2,
    .knockback = vec2(-backattack_f*c->direction*.1f, 0),
    .effect    = loeffect_immediate_damage(c->recipient.status.attack),
  });
}

static void locharacter_big_warder_update_cooldown_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat     = LOCHARACTER_BIG_WARDER_BEAT;
  static const uint64_t duration = beat*4;

  const uint64_t ti = c->cache.time - c->since;

  const locharacter_big_warder_param_t* p = (typeof(p)) c->data;

  c->cache.bullet_hittest = true;
  c->cache.gravity        = true;

  /* ---- motion ---- */
  float t = ti*1.f/duration;
  if (t > 1) t = 1;

  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
  instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  instance->motion_time    = t;

  /* ---- position ---- */
  if (ti < beat*2) {
    t = ti/beat/2;
    vec2_sub(&c->pos, &p->to, &p->from);
    vec2_muleq(&c->pos, t*t*(3-2*t));
    vec2_addeq(&c->pos, &p->from);

    t = t*2-1;
    t = 1-powf(MATH_ABS(t), 2);
    c->pos.y += t*c->recipient.status.jump/2;
  } else {
    c->pos = vec2(0, 0);
  }

  /* ---- state transition ---- */
  if (locharacter_big_warder_reset_if_player_left_(c)) return;

  if (c->since + duration <= c->cache.time) {
    locharacter_big_warder_start_shoot_state_(c);
    return;
  }
}
static void locharacter_big_warder_start_cooldown_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  locharacter_big_warder_param_t* p = (typeof(p)) c->data;

  if (locharacter_event_holder_has_control(&p->event)) {
    static const uint64_t dur = LOCHARACTER_BIG_WARDER_MUSIC_DURATION;
    if (p->event.start_time+dur < c->cache.time) {
      locharacter_big_warder_finalize_event_(c);
      return;
    }
  } else {
    if (c->recipient.madness <= 0) {
      locharacter_big_warder_start_dead_state_(c);
      return;
    }
  }

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_COOLDOWN;

  p->from = c->pos;
  p->to   = vec2(0, 0);
}

static void locharacter_big_warder_update_stunned_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float    beat     = LOCHARACTER_BIG_WARDER_BEAT;
  static const uint64_t duration = beat*4;

  c->cache.gravity = true;

  /* ---- motion ---- */
  float t = (c->cache.time - c->since)*1.f/duration;
  t *= 6;

  loshader_character_drawer_instance_t* instance = &c->cache.instance;
  if (t < 1) {
    t = 1-powf(1-t, 6);
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_DOWN;
    instance->motion_time    = t;
  } else {
    t = (t-1)/5;
    if (t > 1) t = 1;
    t = t*t*(3-2*t);
    instance->from_motion_id = LOSHADER_CHARACTER_MOTION_ID_DOWN;
    instance->to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    instance->motion_time    = t;
  }

  /* ---- state transition ---- */
  if (c->since + duration <= c->cache.time) {
    locharacter_big_warder_start_cooldown_state_(c);
    return;
  }
}
static void locharacter_big_warder_start_stunned_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_STUNNED;

  loeffect_recipient_apply_effect(
      &c->recipient, &loeffect_immediate_damage(1.f));
}

static void locharacter_big_warder_update_dead_state_(
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
  instance->color.w       *= 1-t;

  /* ---- state transition ---- */
  if (c->since+duration < c->cache.time) {
    locharacter_big_warder_start_wait_state_(c);
    return;
  }
}
static void locharacter_big_warder_start_dead_state_(
    locharacter_base_t* c) {
  assert(c != NULL);

  c->since = c->cache.time;
  c->state = LOCHARACTER_STATE_DEAD;

  loplayer_gain_faith(c->player, .8f);
}

bool locharacter_big_warder_update(locharacter_base_t* base) {
  assert(base != NULL);

  static const vec2_t size  = locharacter_big_warder_size_;
  static const float height = size.y * 1.4f;
  static const float drawsz = MATH_MAX(size.x, size.y);

  locharacter_big_warder_param_t* p = (typeof(p)) base->data;

  loeffect_recipient_update(
      &base->recipient, &locharacter_big_warder_base_status_);

  if (!locharacter_event_holder_update(&p->event)) {
    locharacter_big_warder_start_wait_state_(base);
  }

  base->cache.instance = (loshader_character_drawer_instance_t) {
    .character_id  = LOSHADER_CHARACTER_ID_WARDER,
    .marker_offset = vec2(0, height - drawsz),
    .pos           = vec2(0, drawsz - height),
    .size          = vec2(drawsz, drawsz),
    .color         = vec4(.2f, 0, 0, 1),
  };

  switch (base->state) {
  case LOCHARACTER_STATE_WAIT:
    locharacter_big_warder_update_wait_state_(base);
    break;
  case LOCHARACTER_STATE_WALK:
    locharacter_big_warder_update_walk_state_(base);
    break;
  case LOCHARACTER_STATE_SHOOT:
    locharacter_big_warder_update_shoot_state_(base);
    break;
  case LOCHARACTER_STATE_COMBO:
    locharacter_big_warder_update_combo_state_(base);
    break;
  case LOCHARACTER_STATE_THRUST:
    locharacter_big_warder_update_thrust_state_(base);
    break;
  case LOCHARACTER_STATE_COOLDOWN:
    locharacter_big_warder_update_cooldown_state_(base);
    break;
  case LOCHARACTER_STATE_STUNNED:
    locharacter_big_warder_update_stunned_state_(base);
    break;
  case LOCHARACTER_STATE_DEAD:
    locharacter_big_warder_update_dead_state_(base);
    break;
  default:
    locharacter_big_warder_start_wait_state_(base);
  }
  locharacter_big_warder_update_passive_action_(base);

  base->cache.height = height;

  base->cache.instance.marker  = !!base->cache.bullet_hittest;
  base->cache.instance.size.x *= base->direction;
  return true;
}

void locharacter_big_warder_build(
    locharacter_base_t* base, loentity_id_t ground) {
  assert(base != NULL);

  base->type = LOCHARACTER_TYPE_BIG_WARDER;

  base->ground = ground;

  base->pos       = vec2(.7f, 0);
  base->direction = 1;

  base->state = LOCHARACTER_STATE_WAIT;
  base->since = base->cache.time;

  locharacter_big_warder_param_t* p = (typeof(p)) base->data;
  *p = (typeof(*p)) {0};

  locharacter_event_holder_initialize(
      &p->event,
      &base->res->music.boss_big_warder,
      base,
      LOCHARACTER_BIG_WARDER_MUSIC_DURATION,
      0);
}

void locharacter_big_warder_tear_down(locharacter_base_t* base) {
  assert(base != NULL);

  locharacter_big_warder_param_t* p = (typeof(p)) base->data;
  locharacter_event_holder_deinitialize(&p->event);
}

void locharacter_big_warder_pack_data(
    const locharacter_base_t* base, msgpack_packer* packer) {
  assert(base   != NULL);
  assert(packer != NULL);

  const locharacter_big_warder_param_t* p = (typeof(p)) base->data;

  msgpack_pack_map(packer, LOCHARACTER_BIG_WARDER_PARAM_TO_PACK_COUNT);

# define pack_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &p->var);  \
  } while (0)

  LOCHARACTER_BIG_WARDER_PARAM_TO_PACK_EACH_(pack_);

# undef pack_
}

bool locharacter_big_warder_unpack_data(
    locharacter_base_t* base, const msgpack_object* obj) {
  assert(base != NULL);

  locharacter_big_warder_param_t* p = (typeof(p)) base->data;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define unpack_(name, var) do {  \
    if (!LOCOMMON_MSGPACK_UNPACK_ANY(item_(name), &p->var)) {  \
      return false;  \
    }  \
  } while (0)

  LOCHARACTER_BIG_WARDER_PARAM_TO_PACK_EACH_(unpack_);

# undef unpack_

# undef item_

  locharacter_event_holder_initialize(
      &p->event,
      &base->res->music.boss_big_warder,
      base,
      LOCHARACTER_BIG_WARDER_MUSIC_DURATION,
      p->event.start_time);
  return true;
}
