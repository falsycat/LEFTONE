#include "./big_warder.h"

#include <stdbool.h>
#include <stdint.h>

#include "util/chaos/xorshift.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/statman/statman.h"

#include "core/lobullet/linear.h"
#include "core/lobullet/pool.h"
#include "core/locommon/position.h"
#include "core/loentity/ground.h"
#include "core/loplayer/event.h"
#include "core/loplayer/stance.h"
#include "core/loresource/music.h"
#include "core/loshader/character.h"

#include "./base.h"
#include "./state.h"
#include "./strategy.h"
#include "./type.h"

#define WIDTH_  .025f
#define HEIGHT_ .06f
#define MARKER_ .03f
#define COLOR_  vec4(0, 0, 0, 1)

#define BPM_            80
#define BEAT_           (60.f/BPM_)
#define BEAT_MS_        (BEAT_*1000)
#define MUSIC_DURATION_ (BEAT_MS_*144)

#define WAKE_UP_RANGE_ (WIDTH_*8)

#define REWARD_STANCE_ LOPLAYER_STANCE_UNFINISHER

static const loeffect_recipient_status_t base_status_ = {
  .attack  = .2f,
  .defence = .92f,
  .speed   = .31f,
  .jump    = 1.1f,
};

static void lochara_big_warder_initialize_shoot_state_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  lochara_state_initialize_any_(meta, instance, state);

  lochara_base_t* base = instance;

  vec2_t dir;
  locommon_position_sub(
      &dir, &base->player->entity->super.super.pos, &base->super.super.pos);

  if (dir.x == 0 && dir.y == 0) dir = vec2(1, 0);
  vec2_diveq(&dir, vec2_length(&dir));

  const vec2_t invdir = vec2(dir.y, -dir.x);

  base->param.direction = vec2(MATH_SIGN_NOZERO(dir.x), 0);
  for (int32_t i = -4; i <= 4; ++i) {
    vec2_t a;
    vec2_mul(&a, &dir, 1.5f-MATH_ABS(i)/4.f*.5f);

    vec2_t v;
    vec2_mul(&v, &dir, .5f);

    vec2_t p;
    vec2_mul(&p, &invdir, i*.02f);

    locommon_position_t pos = base->super.super.pos;
    vec2_addeq(&pos.fract, &p);
    locommon_position_reduce(&pos);

    lobullet_base_t* b = lobullet_pool_create(base->bullet);
    lobullet_linear_circle_build(b,
          .owner        = base->super.super.id,
          .basepos      = pos,
          .size         = vec2(.02f, .02f),
          .color        = vec4(1, 1, 1, 1),
          .acceleration = a,
          .velocity     = v,
          .knockback    = 1,
          .effect       = loeffect_damage(
              base->param.recipient.status.attack*.6f),
          .duration     = 2000,
        );
    loentity_store_add(base->entities, &b->super.super);
  }
}

static void lochara_big_warder_update_thrust_in_state_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  static const uint64_t dur = BEAT_MS_/4;

  lochara_base_t* base = instance;

  uint64_t t = base->ticker->time - base->param.last_state_changed;
  if (t > dur) t = dur;

  const float tf = t*1.f / dur;
  base->cache.instance.motion.time = powf(tf, 2);
  base->cache.instance.motion.from = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
  base->cache.instance.motion.to   = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
}

static void lochara_big_warder_update_thrust_out_state_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  static const uint64_t dur = BEAT_MS_/4;

  lochara_base_t* base = instance;

  uint64_t t = base->ticker->time - base->param.last_state_changed;
  if (t > dur) t = dur;

  const float tf = t*1.f / dur;
  base->cache.instance.motion.time = 1-powf(1-tf, 2);
  base->cache.instance.motion.from = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
  base->cache.instance.motion.to   = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
}

static void lochara_big_warder_update_down_state_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  const bool fast =
      meta->state == LOCHARA_STATE_DOWN ||
      meta->state == LOCHARA_STATE_REVIVE;
  const bool reverse =
      meta->state == LOCHARA_STATE_REVIVE ||
      meta->state == LOCHARA_STATE_RESUSCITATE;

  const uint64_t dur = fast? reverse? BEAT_MS_*3: BEAT_MS_: BEAT_MS_*4;

  lochara_base_t* base = instance;
  base->param.movement = vec2(0, 0);

  uint64_t t = base->ticker->time - base->param.last_state_changed;
  if (t > dur) t = dur;
  if (reverse) t = dur - t;

  base->cache.instance.motion.from = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  base->cache.instance.motion.to   = LOSHADER_CHARACTER_MOTION_ID_DOWN;
  base->cache.instance.motion.time = powf(t*1.f/dur, 6);
}

static const statman_meta_t state_table_[] = {
  lochara_state_stand(
        .period       = BEAT_MS_*2,
        .acceleration = {{5, 5}},
        .motion1      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
        .motion2      = LOSHADER_CHARACTER_MOTION_ID_ATTACK1,
      ),
  lochara_state_walk(
        .period       = BEAT_MS_,
        .acceleration = {{5, 5}},
        .motion1      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
        .motion2      = LOSHADER_CHARACTER_MOTION_ID_WALK,
      ),
  lochara_state_dodge(
        .duration     = BEAT_MS_/2,
        .speed        = 1,
        .acceleration = {{3, 3}},
        .motion1      = LOSHADER_CHARACTER_MOTION_ID_WALK,
        .motion2      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
      ),

  lochara_state_teleport(
        .duration = BEAT_MS_,
        .offset   = {{WIDTH_*2, 0}},
        .motion1  = LOSHADER_CHARACTER_MOTION_ID_STAND1,
        .motion2  = LOSHADER_CHARACTER_MOTION_ID_ATTACK2,
      ),

  {
    .state      = LOCHARA_STATE_THRUST_IN,
    .name       = "THRUST_IN",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_big_warder_update_thrust_in_state_,
  },
  {
    .state      = LOCHARA_STATE_THRUST_OUT,
    .name       = "THRUST_OUT",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_big_warder_update_thrust_out_state_,
  },
  {
    .state      = LOCHARA_STATE_SHOOT,
    .name       = "SHOOT",
    .initialize = lochara_big_warder_initialize_shoot_state_,
    .update     = lochara_big_warder_update_thrust_in_state_,
  },
  {
    .state      = LOCHARA_STATE_DOWN,
    .name       = "DOWN",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_big_warder_update_down_state_,
  },
  {
    .state      = LOCHARA_STATE_REVIVE,
    .name       = "REVIVE",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_big_warder_update_down_state_,
  },
  {
    .state      = LOCHARA_STATE_DEAD,
    .name       = "DEAD",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_big_warder_update_down_state_,
  },
  {
    .state      = LOCHARA_STATE_RESUSCITATE,
    .name       = "RESUSCITATE",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_big_warder_update_down_state_,
  },
  {0},
};

static void lochara_big_warder_update_wait_strategy_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  static const float range2 = WAKE_UP_RANGE_*WAKE_UP_RANGE_;

  lochara_base_t* base = instance;

  vec2_t disp;
  locommon_position_sub(
      &disp, &base->player->entity->super.super.pos, &base->super.super.pos);

  if (fabsf(disp.y) < HEIGHT_/2 && vec2_pow_length(&disp) < range2) {
    loeffect_recipient_apply_effect(
        &base->param.recipient, &loeffect_resuscitate());
    if (loplayer_stance_set_has(&base->player->stances, REWARD_STANCE_)) {
      *next = LOCHARA_STRATEGY_WAKE_UP;
    } else {
      *next = LOCHARA_STRATEGY_WAKE_UP_EVENT;
    }
    return;
  }
  statman_transition_to(
      state_table_, instance, &base->param.state, LOCHARA_STATE_DEAD);
}

static void lochara_big_warder_initialize_wake_up_strategy_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  lochara_strategy_initialize_any_(meta, instance, next);
  if (meta->state != LOCHARA_STRATEGY_WAKE_UP_EVENT) return;

  lochara_base_t* base = instance;
  loentity_character_apply_effect(
      &base->super, &loeffect_fanatic(MUSIC_DURATION_));
  loentity_character_apply_effect(
      &base->player->entity->super, &loeffect_curse(MUSIC_DURATION_));
}

static void lochara_big_warder_update_wake_up_strategy_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  const bool     ev  = meta->state == LOCHARA_STRATEGY_WAKE_UP_EVENT;
  const uint64_t dur = (ev? BEAT_MS_*16: BEAT_MS_*4);

  lochara_base_t* base = instance;

  const uint64_t t = base->ticker->time - base->param.last_strategy_changed;
  if (t >= dur) {
    *next = LOCHARA_STRATEGY_APPROACH;
    return;
  }
  statman_transition_to(
      state_table_, instance, &base->param.state, LOCHARA_STATE_RESUSCITATE);
}

static void lochara_big_warder_update_approach_strategy_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  lochara_base_t* base = instance;
  if (!loeffect_recipient_is_alive(&base->param.recipient)) {
    *next = LOCHARA_STRATEGY_DEAD;
    return;
  }

  const uint64_t since = base->param.last_strategy_changed;

  uint64_t until = since + BEAT_MS_;
  if (base->player->event.executor == base->super.super.id) {
    const uint64_t msince = base->player->event.ctx.music.since;
    if (msince < since) {
      const uint64_t beats = (since - msince)/BEAT_MS_ + 1;
      until = msince + beats*BEAT_MS_;
    }
  }

  /* ---- strategy transition ---- */
  const locommon_position_t* player = &base->player->entity->super.super.pos;

  vec2_t disp;
  locommon_position_sub(&disp, player, &base->super.super.pos);

  if (player->chunk.x != base->super.super.pos.chunk.x ||
      player->chunk.y != base->super.super.pos.chunk.y ||
      disp.y < -HEIGHT_) {
    *next = LOCHARA_STRATEGY_WAIT;
    return;
  }

  const float dist = MATH_ABS(disp.x);
  if (base->ticker->time >= until) {
    if (MATH_ABS(disp.y) > HEIGHT_) {
      *next = LOCHARA_STRATEGY_SHOOT1;
    } else if (dist < WIDTH_*5) {
      *next = LOCHARA_STRATEGY_COMBO1;
      if (chaos_xorshift(base->ticker->time)%3 == 0) {
        *next = LOCHARA_STRATEGY_COMBO2;
      }
    } else if (dist < WIDTH_*10) {
      *next = LOCHARA_STRATEGY_COMBO2;
    } else {
      *next = LOCHARA_STRATEGY_SHOOT1;
    }
    return;
  }

  if (dist > WIDTH_*6) {
    base->param.direction = vec2(MATH_SIGN_NOZERO(disp.x), 0);
  }
}

static const lochara_combat_action_t combo1_[] = {
  lochara_combat_action_rest(
        .duration = BEAT_MS_/2,
        .state    = LOCHARA_STATE_THRUST_IN,
      ),
  lochara_combat_action_attack(
        .duration = BEAT_MS_/2,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_,
        .state    = LOCHARA_STATE_THRUST_OUT,
      ),
  lochara_combat_action_attack(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_THRUST_OUT,
      ),
  lochara_combat_action_attack(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1.2f,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_DODGE_LEFT,
      ),
  {0},
};

static const lochara_combat_action_t combo2_[] = {
  lochara_combat_action_rest(
        .duration = BEAT_MS_,
        .state    = LOCHARA_STATE_TELEPORT_FRONT,
      ),
  lochara_combat_action_attack(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_THRUST_OUT,
      ),
  lochara_combat_action_attack(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_THRUST_OUT,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/2,
        .state    = LOCHARA_STATE_THRUST_IN,
      ),
  lochara_combat_action_attack(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_DODGE_RIGHT,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_,
        .state    = LOCHARA_STATE_TELEPORT_BEHIND,
      ),
  {0},
};

static const lochara_combat_action_t combo3_[] = {
  lochara_combat_action_rest(
        .duration = BEAT_MS_,
        .state    = LOCHARA_STATE_THRUST_IN,
      ),
  lochara_combat_action_attack(
        .duration = BEAT_MS_/2,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/2,
        .state    = LOCHARA_STATE_THRUST_OUT,
      ),
  lochara_combat_action_attack(
        .duration = BEAT_MS_/2,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/2,
        .state    = LOCHARA_STATE_THRUST_OUT,
      ),
  lochara_combat_action_attack(
        .duration = BEAT_MS_/2,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_THRUST_OUT,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_DODGE_RIGHT,
      ),
  {0},
};

static const lochara_combat_action_t shoot1_[] = {
  lochara_combat_action_rest(
        .duration = BEAT_MS_,
        .state    = LOCHARA_STATE_SHOOT,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_DODGE_RIGHT,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_,
        .state    = LOCHARA_STATE_SHOOT,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/4,
        .state    = LOCHARA_STATE_DODGE_LEFT,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_/2*3,
        .state    = LOCHARA_STATE_STAND,
      ),
  {0},
};

static const lochara_combat_action_t down_[] = {
  lochara_combat_action_rest(
        .duration = BEAT_MS_,
        .state    = LOCHARA_STATE_DOWN,
      ),
  lochara_combat_action_rest(
        .duration = BEAT_MS_*3,
        .state    = LOCHARA_STATE_REVIVE,
      ),
  {0},
};

static const lochara_combat_action_t kill_[] = {
  lochara_combat_action_rest(
        .duration = BEAT_MS_*4,
        .state    = LOCHARA_STATE_DEAD,
      ),
  {0},
};

static const lochara_combat_action_t dead_[] = {
  lochara_combat_action_rest(
        .duration = 30000,
        .state    = LOCHARA_STATE_DEAD,
      ),
  {0},
};

static const statman_meta_t strategy_table_[] = {
  {
    .state      = LOCHARA_STRATEGY_WAIT,
    .name       = "WAIT",
    .initialize = lochara_strategy_initialize_any_,
    .update     = lochara_big_warder_update_wait_strategy_,
  },
  {
    .state      = LOCHARA_STRATEGY_WAKE_UP,
    .name       = "WAKE_UP",
    .initialize = lochara_strategy_initialize_any_,
    .update     = lochara_big_warder_update_wake_up_strategy_,
  },
  {
    .state      = LOCHARA_STRATEGY_WAKE_UP_EVENT,
    .name       = "WAKE_UP_EVENT",
    .initialize = lochara_big_warder_initialize_wake_up_strategy_,
    .update     = lochara_big_warder_update_wake_up_strategy_,
  },
  {
    .state      = LOCHARA_STRATEGY_APPROACH,
    .name       = "APPROACH",
    .initialize = lochara_strategy_initialize_any_,
    .update     = lochara_big_warder_update_approach_strategy_,
  },

  lochara_strategy_combat(COMBO1,
      .state_table  = state_table_,
      .actions      = combo1_,
      .parry_window = 100,
      .parried_next = LOCHARA_STRATEGY_COMBO3,
      .next         = LOCHARA_STRATEGY_APPROACH,
    ),
  lochara_strategy_combat(COMBO2,
      .state_table  = state_table_,
      .actions      = combo2_,
      .parry_window = 100,
      .parried_next = LOCHARA_STRATEGY_DOWN,
      .next         = LOCHARA_STRATEGY_COMBO3,
    ),
  lochara_strategy_combat(COMBO3,
      .state_table  = state_table_,
      .actions      = combo3_,
      .parried_next = LOCHARA_STRATEGY_DOWN,
      .next         = LOCHARA_STRATEGY_APPROACH,
    ),
  lochara_strategy_combat(SHOOT1,
      .state_table  = state_table_,
      .actions      = shoot1_,
      .parry_window = 200,
      .parried_next = LOCHARA_STRATEGY_DOWN,
      .gravity      = true,
      .next         = LOCHARA_STRATEGY_APPROACH,
    ),

  lochara_strategy_combat(DOWN,
      .state_table = state_table_,
      .actions     = down_,
      .gravity     = true,
      .next        = LOCHARA_STRATEGY_APPROACH,
    ),
  lochara_strategy_combat(KILL,
      .state_table = state_table_,
      .actions     = kill_,
      .gravity     = true,
      .next        = LOCHARA_STRATEGY_WAIT,
    ),
  lochara_strategy_combat(DEAD,
      .state_table = state_table_,
      .actions     = dead_,
      .gravity     = true,
      .next        = LOCHARA_STRATEGY_WAIT,
    ),
  {0},
};

static void lochara_big_warder_update_event_(lochara_base_t* base) {
  assert(base != NULL);

  static const loplayer_event_command_t wake_up[] = {
    loplayer_event_command_play_music(LORESOURCE_MUSIC_ID_BOSS_BIG_WARDER),
    loplayer_event_command_set_area(.49f, .45f),
    loplayer_event_command_set_cinescope(1),
    loplayer_event_command_wait(BEAT_MS_*4),

    loplayer_event_command_set_line("boss_big_warder_line0"),
    loplayer_event_command_wait(BEAT_MS_*8),
    loplayer_event_command_set_line("boss_big_warder_line1"),
    loplayer_event_command_wait(BEAT_MS_*4),

    loplayer_event_command_set_line(NULL),
    loplayer_event_command_set_cinescope(0),
    {0},
  };
  static const loplayer_event_command_t kill[] = {
    loplayer_event_command_set_area(0, 0),
    loplayer_event_command_set_cinescope(1),
    loplayer_event_command_wait(BEAT_MS_),
    loplayer_event_command_stop_music(),

    loplayer_event_command_set_line("boss_big_warder_kill_line"),
    loplayer_event_command_wait(BEAT_MS_*4),

    loplayer_event_command_finalize(),
    {0},
  };
  static const loplayer_event_command_t dead[] = {
    loplayer_event_command_set_area(0, 0),
    loplayer_event_command_wait(BEAT_MS_),
    loplayer_event_command_stop_music(),
    loplayer_event_command_finalize(),
    {0},
  };

  const uint64_t t = base->ticker->time;

  loplayer_event_t*   event = &base->player->event;
  const loentity_id_t id    = base->super.super.id;

  locommon_position_t basepos = base->super.super.pos;
  basepos.fract = vec2(.5f, .5f);

  if (base->param.strategy == LOCHARA_STRATEGY_WAKE_UP_EVENT) {
    loplayer_event_execute_commands(event, id, &basepos, wake_up);
    return;
  }
  if (event->executor != id) return;

  if (base->player->entity->param.state == LOCHARA_STATE_DEAD) {
    statman_transition_to(
        strategy_table_, base, &base->param.strategy, LOCHARA_STRATEGY_KILL);
    loplayer_event_execute_commands(event, id, &basepos, kill);
    return;
  }
  if (base->param.strategy == LOCHARA_STRATEGY_DEAD) {
    loplayer_event_execute_commands(event, id, &basepos, dead);
    if (!loplayer_stance_set_has(&base->player->stances, REWARD_STANCE_)) {
      loplayer_stance_set_add(&base->player->stances, REWARD_STANCE_);
      loplayer_popup_queue_new_stance(&base->player->popup, REWARD_STANCE_);
    }
    return;
  }
  if (base->player->event.basetime+MUSIC_DURATION_ <= t &&
      loeffect_recipient_is_alive(&base->param.recipient)) {
    loentity_character_apply_effect(
        &base->player->entity->super, &loeffect_curse_trigger());
    return;
  }
  loplayer_event_execute_commands(event, id, &basepos, wake_up);
}

bool lochara_big_warder_update(lochara_base_t* base) {
  assert(base != NULL);

  loeffect_recipient_update(&base->param.recipient, &base_status_);

  statman_update(strategy_table_, base, &base->param.strategy);
  lochara_big_warder_update_event_(base);

  const float dir = MATH_SIGN_NOZERO(base->param.direction.x);
  base->cache.instance = (loshader_character_drawer_instance_t) {
    .character_id  = LOSHADER_CHARACTER_ID_WARDER,
    .pos           = vec2(0, -MARKER_),
    .size          = vec2(dir*HEIGHT_, HEIGHT_),
    .color         = COLOR_,
    .marker_offset = vec2(0, MARKER_),
  };
  statman_update(state_table_, base, &base->param.state);

  if (base->param.strategy != LOCHARA_STRATEGY_WAIT          &&
      base->param.strategy != LOCHARA_STRATEGY_WAKE_UP       &&
      base->param.strategy != LOCHARA_STRATEGY_WAKE_UP_EVENT &&
      base->param.strategy != LOCHARA_STRATEGY_DEAD) {
    base->cache.instance.marker = lochara_base_affect_bullets(base);
  }

  lochara_base_calculate_physics(
      base, &vec2(WIDTH_, HEIGHT_), &vec2(0, MARKER_));
  lochara_base_bind_on_ground(base, &vec2(WIDTH_, HEIGHT_+MARKER_));
  return true;
}

void lochara_big_warder_build(
    lochara_base_t* base, loentity_ground_t* gnd) {
  assert(base != NULL);
  assert(gnd  != NULL);

  base->super.super.pos = gnd->super.pos;
  vec2_addeq(&base->super.super.pos.fract, &vec2(0, gnd->size.y));
  locommon_position_reduce(&base->super.super.pos);

  base->param = (typeof(base->param)) {
    .type                  = LOCHARA_TYPE_BIG_WARDER,
    .state                 = LOCHARA_STATE_DEAD,
    .last_state_changed    = base->ticker->time,
    .strategy              = LOCHARA_STRATEGY_WAIT,
    .last_strategy_changed = base->ticker->time,

    .ground = gnd->super.id,
  };
  loeffect_recipient_initialize(
      &base->param.recipient, base->ticker, &base_status_);
}
