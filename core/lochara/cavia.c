#include "./cavia.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/statman/statman.h"

#include "core/lobullet/base.h"
#include "core/lobullet/linear.h"
#include "core/loeffect/effect.h"
#include "core/loeffect/recipient.h"
#include "core/loentity/ground.h"
#include "core/loresource/sound.h"
#include "core/loshader/character.h"

#include "./base.h"
#include "./combat.h"

#define WIDTH_  .02f
#define HEIGHT_ .05f
#define MARKER_ .02f
#define COLOR_  vec4(0, 0, 0, 1)

static const loeffect_recipient_status_t base_status_ = {
  .attack  = .1f,
  .defence = .3f,
  .speed   = .05f,
  .jump    = 1.f,
};

static void lochara_cavia_update_thrust_in_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  static const uint64_t dur = 100;

  lochara_base_t* base = instance;

  uint64_t t = base->ticker->time - base->param.last_state_changed;
  if (t > dur) t = dur;

  const float tf = t*1.f / dur;
  base->cache.instance.motion.time = powf(tf, 2);
  base->cache.instance.motion.from = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
  base->cache.instance.motion.to   = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
}

static void lochara_cavia_update_thrust_out_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  static const uint64_t dur = 100;

  lochara_base_t* base = instance;

  uint64_t t = base->ticker->time - base->param.last_state_changed;
  if (t > dur) t = dur;

  const float tf = t*1.f / dur;
  base->cache.instance.motion.time = powf(tf, 2);
  base->cache.instance.motion.from = LOSHADER_CHARACTER_MOTION_ID_ATTACK2;
  base->cache.instance.motion.to   = LOSHADER_CHARACTER_MOTION_ID_ATTACK1;
}

static void lochara_cavia_initialize_dead_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  lochara_state_initialize_any_(meta, instance, state);

  lochara_base_t* base = instance;
  for (size_t i = 0; i < 12; ++i) {
    const float t = MATH_PI/6*i;

    vec2_t v, a, p;
    p = vec2(cos(t), sin(t));
    vec2_mul(&v, &p, .7f);
    vec2_mul(&a, &p, -1.4f);
    vec2_muleq(&p, .1f);

    locommon_position_t pos = base->super.super.pos;
    vec2_addeq(&pos.fract, &p);
    locommon_position_reduce(&pos);

    lobullet_base_t* b = lobullet_pool_create(base->bullet);
    lobullet_linear_triangle_build(b,
          .owner        = b->super.super.id,
          .basepos      = pos,
          .velocity     = v,
          .acceleration = a,
          .size         = vec2(.01f, .045f),
          .color        = vec4(0, 0, 0, .8f),
          .angle        = t-MATH_PI,
          .knockback    = 1,
          .effect       = loeffect_damage(.1f),
          .duration     = 1200,
        );
    loentity_store_add(base->entities, &b->super.super);
  }
  loresource_sound_set_play(&base->res->sound, LORESOURCE_SOUND_ID_ENEMY_TRIGGER);
}

static void lochara_cavia_update_dead_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  lochara_base_t* base = instance;
  base->param.movement = vec2(0, 0);

  uint64_t t = base->ticker->time - base->param.last_state_changed;
  if (t > 2000) {
    t = 2000;
  }
  if (meta->state == LOCHARA_STATE_RESUSCITATE) {
    if (t == 0) {
      loeffect_recipient_apply_effect(
          &base->param.recipient, &loeffect_resuscitate());
    }
    t = 2000 - t;
  }

  if (t < 1000) {
    const float tf = t/1000.f;
    base->cache.instance.motion.from = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    base->cache.instance.motion.to   = LOSHADER_CHARACTER_MOTION_ID_SIT;
    base->cache.instance.motion.time = powf(tf, 6);
  } else {
    const float tf = (t-1000)/1000.f;
    base->cache.instance.motion.from = LOSHADER_CHARACTER_MOTION_ID_SIT;
    base->cache.instance.motion.to   = LOSHADER_CHARACTER_MOTION_ID_DOWN;
    base->cache.instance.motion.time = powf(tf, 4);
  }
}

static const statman_meta_t state_table_[] = {
  lochara_state_stand(
        .period       = 4000,
        .acceleration = {{5, 5}},
        .motion1      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
        .motion2      = LOSHADER_CHARACTER_MOTION_ID_STAND2,
      ),
  lochara_state_walk(
        .period       = 1000,
        .acceleration = {{5, 5}},
        .motion1      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
        .motion2      = LOSHADER_CHARACTER_MOTION_ID_WALK,
      ),
  {
    .state      = LOCHARA_STATE_THRUST_IN,
    .name       = "THRUST_IN",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_cavia_update_thrust_in_,
  },
  {
    .state      = LOCHARA_STATE_THRUST_OUT,
    .name       = "THRUST_OUT",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_cavia_update_thrust_out_,
  },
  {
    .state      = LOCHARA_STATE_DEAD,
    .name       = "DEAD",
    .initialize = lochara_cavia_initialize_dead_,
    .update     = lochara_cavia_update_dead_,
  },
  {
    .state      = LOCHARA_STATE_RESUSCITATE,
    .name       = "RESUSCITATE",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_cavia_update_dead_,
  },
  {0},
};

static const lochara_combat_action_t combo1_[] = {
  lochara_combat_action_rest(
        .duration = 300,
        .state    = LOCHARA_STATE_STAND,
      ),
  lochara_combat_action_attack(
        .duration = 300,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1.2f,
      ),
  lochara_combat_action_rest(
        .duration = 100,
        .state    = LOCHARA_STATE_THRUST_OUT,
      ),
  lochara_combat_action_attack(
        .duration = 150,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = .8f,
      ),
  lochara_combat_action_rest(
        .duration = 150,
        .state    = LOCHARA_STATE_THRUST_OUT,
      ),
  lochara_combat_action_attack(
        .duration = 150,
        .state    = LOCHARA_STATE_THRUST_IN,
        .damage   = 1,
      ),
  lochara_combat_action_rest(
        .duration = 1000,
        .state    = LOCHARA_STATE_THRUST_OUT,
      ),
  {0},
};

static const lochara_combat_action_t dead_[] = {
  lochara_combat_action_rest(
        .duration = 30000,
        .state    = LOCHARA_STATE_DEAD,
      ),
  lochara_combat_action_rest(
        .duration = 2000,
        .state    = LOCHARA_STATE_RESUSCITATE,
      ),
  {0},
};

static const statman_meta_t strategy_table_[] = {
  lochara_strategy_scouting(
        .state_table = state_table_,
        .period      = 2000,
        .stagger     = .5f,
        .range_close = WIDTH_*2,
        .found_close = LOCHARA_STRATEGY_COMBO1,
        .range_back  = 1,
      ),
  lochara_strategy_combat(COMBO1,
        .state_table = state_table_,
        .actions     = combo1_,
        .next        = LOCHARA_STRATEGY_SCOUTING,
      ),
  lochara_strategy_combat(DEAD,
        .state_table = state_table_,
        .actions     = dead_,
        .next        = LOCHARA_STRATEGY_SCOUTING,
      ),
  {0},
};

bool lochara_cavia_update(lochara_base_t* base) {
  assert(base != NULL);

  loeffect_recipient_update(&base->param.recipient, &base_status_);

  statman_update(strategy_table_, base, &base->param.strategy);

  const float dir = MATH_SIGN_NOZERO(base->param.direction.x);
  base->cache.instance = (loshader_character_drawer_instance_t) {
    .character_id  = LOSHADER_CHARACTER_ID_CAVIA,
    .pos           = vec2(0, -MARKER_),
    .size          = vec2(dir*HEIGHT_, HEIGHT_),
    .color         = COLOR_,
    .marker_offset = vec2(0, MARKER_),
  };
  statman_update(state_table_, base, &base->param.state);

  if (base->param.state != LOCHARA_STATE_DEAD) {
    base->cache.instance.marker = lochara_base_affect_bullets(base);
  }

  lochara_base_calculate_physics(
      base, &vec2(WIDTH_, HEIGHT_), &vec2(0, MARKER_));
  lochara_base_bind_on_ground(base, &vec2(WIDTH_, HEIGHT_+MARKER_));
  return true;
}

void lochara_cavia_build(
    lochara_base_t* base, const loentity_ground_t* gnd, float pos) {
  assert(base != NULL);
  assert(gnd  != NULL);
  assert(MATH_FLOAT_VALID(pos));

  base->super.super.pos = gnd->super.pos;
  vec2_addeq(
      &base->super.super.pos.fract,
      &vec2(gnd->size.x*pos, gnd->size.y));
  locommon_position_reduce(&base->super.super.pos);

  base->param = (typeof(base->param)) {
    .type                  = LOCHARA_TYPE_CAVIA,
    .state                 = LOCHARA_STATE_STAND,
    .last_state_changed    = base->ticker->time,
    .strategy              = LOCHARA_STRATEGY_SCOUTING,
    .last_strategy_changed = base->ticker->time,

    .ground = gnd->super.id,
  };
  loeffect_recipient_initialize(
      &base->param.recipient, base->ticker, &base_status_);
}
