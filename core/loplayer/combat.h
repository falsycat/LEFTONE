#pragma once

#include <stdbool.h>

#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"

typedef enum {
  LOPLAYER_COMBAT_ATTACK_RESULT_EXECUTED,
  LOPLAYER_COMBAT_ATTACK_RESULT_REFLECTED,
  LOPLAYER_COMBAT_ATTACK_RESULT_DODGED,
  LOPLAYER_COMBAT_ATTACK_RESULT_ABORTED,
} loplayer_combat_attack_result_t;

typedef struct loplayer_combat_attack_t loplayer_combat_attack_t;
struct loplayer_combat_attack_t {
  loentity_id_t attacker;

  uint64_t start;
  uint64_t duration;

  void* data1;
  void* data2;

  void
  (*handle)(
      const loplayer_combat_attack_t* attack,
      loplayer_combat_attack_result_t result
  );
};

typedef struct {
  const locommon_ticker_t* ticker;
  loentity_store_t*        entities;

# define LOPLAYER_COMBAT_RESERVE 32
  loplayer_combat_attack_t attacks[LOPLAYER_COMBAT_RESERVE];

  loplayer_combat_attack_t* first_attack;
  loplayer_combat_attack_t* last_attack;

  bool accepting;

  uint64_t last_guard_start;
  uint64_t last_guard_end;
} loplayer_combat_t;

void
loplayer_combat_initialize(
    loplayer_combat_t*       combat,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities
);

void
loplayer_combat_deinitialize(
    loplayer_combat_t* combat
);

/* All added attacks must finish with being called handle function
   while the attacker is alive. */
bool
loplayer_combat_add_attack(
    loplayer_combat_t*              combat,
    const loplayer_combat_attack_t* attack
);

void
loplayer_combat_drop_dead_attack(
    loplayer_combat_t* combat
);

void
loplayer_combat_drop_all_attack(
    loplayer_combat_t*              combat,
    loplayer_combat_attack_result_t reason
);

bool
loplayer_combat_pop_attack(
    loplayer_combat_t*        combat,
    loplayer_combat_attack_t* attack,
    float*                    guard_ratio
);

void
loplayer_combat_set_accepting(
    loplayer_combat_t*              combat,
    bool                            accepting,
    loplayer_combat_attack_result_t reason
);

void
loplayer_combat_set_guarding(
    loplayer_combat_t* combat,
    bool               guarding
);

bool
loplayer_combat_is_attack_pending(
    const loplayer_combat_t* combat
);

void
loplayer_combat_attack_handle(
    const loplayer_combat_attack_t* attack,
    loplayer_combat_attack_result_t result
);
