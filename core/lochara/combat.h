#pragma once

#include <stdint.h>

#include "core/loeffect/effect.h"

#include "./state.h"

typedef struct lochara_base_t lochara_base_t;

typedef enum {
  LOCHARA_COMBAT_ACTION_TYPE_REST,
  LOCHARA_COMBAT_ACTION_TYPE_ATTACK,
  LOCHARA_COMBAT_ACTION_TYPE_ATTACK_EFFECT,
} lochara_combat_action_type_t;

typedef struct {
  lochara_combat_action_type_t type;
  lochara_state_t              state;
  uint64_t                     duration;

  union {
    float      damage;
    loeffect_t effect;
  };
} lochara_combat_action_t;

#define lochara_combat_action_rest(...) {  \
  .type = LOCHARA_COMBAT_ACTION_TYPE_REST,  \
  __VA_ARGS__  \
}
#define lochara_combat_action_attack(...) {  \
  .type = LOCHARA_COMBAT_ACTION_TYPE_ATTACK,  \
  __VA_ARGS__  \
}
#define lochara_combat_action_attack_effect(...) {  \
  .type = LOCHARA_COMBAT_ACTION_TYPE_ATTACK_EFFECT,  \
  __VA_ARGS__  \
}

void
lochara_combat_action_execute_all_attacks(
    const lochara_combat_action_t* actions,
    lochara_base_t*                attacker
);

const lochara_combat_action_t*  /* NULLABLE */
lochara_combat_action_find_by_time(
    const lochara_combat_action_t* actions,
    uint64_t                       t
);
