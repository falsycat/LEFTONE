#include "./combat.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/algorithm.h"

#include "core/loeffect/effect.h"
#include "core/loentity/character.h"
#include "core/loplayer/combat.h"

#include "./base.h"
#include "./state.h"

static void lochara_combat_action_handle_attack_(
    const loplayer_combat_attack_t* attack,
    loplayer_combat_attack_result_t result) {
  assert(attack != NULL);

  lochara_base_t* attacker = attack->data1;
  /* TODO(catfoot): store the result in the attacker */
  if (result != LOPLAYER_COMBAT_ATTACK_RESULT_EXECUTED) return;

  const float v = attacker->param.recipient.status.attack*
      MATH_SIGN_NOZERO(attacker->param.direction.x);
  loentity_character_knockback(&attacker->player->entity->super, &vec2(v, 0));

  const lochara_combat_action_t* action = attack->data2;

  switch (action->type) {
  case LOCHARA_COMBAT_ACTION_TYPE_REST:
    break;
  case LOCHARA_COMBAT_ACTION_TYPE_ATTACK:
    loentity_character_apply_effect(
        &attacker->player->entity->super,
        &loeffect_damage(
            action->damage*attacker->param.recipient.status.attack));
    break;
  case LOCHARA_COMBAT_ACTION_TYPE_ATTACK_EFFECT:
    loentity_character_apply_effect(
        &attacker->player->entity->super, &action->effect);
    break;
  }
}

void lochara_combat_action_execute_all_attacks(
    const lochara_combat_action_t* actions, lochara_base_t* attacker) {
  assert(actions  != NULL);
  assert(attacker != NULL);

  loplayer_combat_t* combat = &attacker->player->combat;

  uint64_t t = attacker->ticker->time;
  for (; actions->duration != 0; t += actions->duration, ++actions) {
    if (actions->type == LOCHARA_COMBAT_ACTION_TYPE_REST) continue;
    loplayer_combat_add_attack(
        combat,
        &(loplayer_combat_attack_t) {
          .attacker = attacker->super.super.id,
          .start    = t,
          .duration = actions->duration,
          .data1    = attacker,
          .data2    = (void*) actions,
          .handle   = lochara_combat_action_handle_attack_,
        });
  }
}

const lochara_combat_action_t* lochara_combat_action_find_by_time(
    const lochara_combat_action_t* actions, uint64_t time) {
  assert(actions != NULL);

  while (actions->duration != 0 && actions->duration <= time) {
    time -= actions->duration;
    ++actions;
  }
  return actions->duration > 0? actions: NULL;
}
