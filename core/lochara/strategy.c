#include "./strategy.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/chaos/xorshift.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/statman/statman.h"

#include "core/loeffect/effect.h"
#include "core/loeffect/recipient.h"

#include "./base.h"
#include "./combat.h"
#include "./state.h"

#define SEED_ 3467

#define PARRY_DAMAGE_ 1

void lochara_strategy_initialize_any_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  lochara_base_t* base = instance;
  base->param.last_strategy_changed = base->ticker->time;
}

void lochara_strategy_cancel_transition_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  *next = meta->state;
}

void lochara_strategy_initialize_scouting_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  lochara_strategy_initialize_any_(meta, instance, next);

  const lochara_strategy_scouting_param_t* p = meta->data;

  lochara_base_t* base = instance;
  statman_transition_to(
      p->state_table, base, &base->param.state, LOCHARA_STATE_STAND);
}

void lochara_strategy_update_scouting_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  const lochara_strategy_scouting_param_t* p = meta->data;
  assert(p->state_table != NULL);
  assert(p->period > 0);

  lochara_base_t* base = instance;

  const uint64_t t = base->ticker->time;

  const bool knockback = base->param.last_knockback+500 > t;

  const bool event = base->player->event.executor != 0;

  /* ---- die ---- */
  if (!loeffect_recipient_is_alive(&base->param.recipient)) {
    *next = LOCHARA_STRATEGY_DEAD;
    return;
  }

  /* ---- strategy transition ---- */
  vec2_t disp;
  locommon_position_sub(
      &disp, &base->player->entity->super.super.pos, &base->super.super.pos);

  if (!knockback && MATH_ABS(disp.y) < .1f && !event) {
    disp.x *= MATH_SIGN_NOZERO(base->param.direction.x);

    const float b = p->range_back;
    if (-p->range_close*b < disp.x && disp.x < p->range_close) {
      *next = p->found_close;
    } else if (-p->range_mid*b < disp.x && disp.x < p->range_mid) {
      *next = p->found_mid;
    } else if (-p->range_long*b < disp.x && disp.x < p->range_long) {
      *next = p->found_long;
    }
    if (*next != LOCHARA_STRATEGY_SCOUTING) return;
  }

  /* ---- state transition ---- */
  const uint64_t since = base->param.last_state_changed;

  uint64_t seed = 1+since*SEED_;
# define rand_() (seed = chaos_xorshift(seed))

  const uint64_t dur = rand_()%p->period + p->period/2;

  lochara_state_t state = base->param.state;
  if (knockback) {
    state = LOCHARA_STATE_STAND;

  } else if (since+dur < t) {
    seed = 1+t*SEED_;
    if (state == LOCHARA_STATE_STAND && rand_()%100/100.f < p->stagger) {
      const float r = .5f + base->cache.ground_pos.x*.5f;
      if (rand_()%100/100.f < r) {
        base->param.direction = vec2(-1, 0);
        state = LOCHARA_STATE_WALK_LEFT;
      } else {
        base->param.direction = vec2(1, 0);
        state = LOCHARA_STATE_WALK_RIGHT;
      }
    } else {
      state = LOCHARA_STATE_STAND;
    }
  }
  statman_transition_to(p->state_table, base, &base->param.state, state);

# undef rand_
}

void lochara_strategy_initialize_combat_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  lochara_base_t* base = instance;

  const lochara_strategy_combat_param_t* p = meta->data;
  assert(p->actions != NULL);

  if (base->param.last_knockback+p->parry_window > base->ticker->time) {
    loentity_character_apply_effect(
        &base->super, &loeffect_damage(PARRY_DAMAGE_));
    *next = p->parried_next;
    return;
  }

  lochara_strategy_initialize_any_(meta, instance, next);

  lochara_combat_action_execute_all_attacks(p->actions, base);

  vec2_t disp;
  locommon_position_sub(
      &disp, &base->player->entity->super.super.pos, &base->super.super.pos);
  base->param.direction = vec2(MATH_SIGN_NOZERO(disp.x), 0);
}

void lochara_strategy_update_combat_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  const lochara_strategy_combat_param_t* p = meta->data;
  assert(p->actions != NULL);
  assert(p->next    != meta->state);

  lochara_base_t* base = instance;
  if (!p->gravity) base->param.gravity = 0;

  const uint64_t t = base->ticker->time - base->param.last_strategy_changed;

  const lochara_combat_action_t* action =
      lochara_combat_action_find_by_time(p->actions, t);
  if (action == NULL) {
    *next = p->next;
    return;
  }
  statman_transition_to(
      p->state_table, base, &base->param.state, action->state);
}
