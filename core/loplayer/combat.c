#include "./combat.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/math/algorithm.h"

#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"

static float loplayer_combat_calc_guard_ratio_(
    const loplayer_combat_t* combat, const loplayer_combat_attack_t* attack) {
  assert(combat != NULL);
  assert(attack != NULL);

  if (combat->last_guard_start >= combat->last_guard_end) {
    return 0;
  }
  const uint64_t st = attack->start;
  const uint64_t ed = st + attack->duration;

  float r = 0;
  r += MATH_DIFF(combat->last_guard_start, st)/1000.f;
  r += MATH_DIFF(combat->last_guard_end,   ed)/1000.f;
  return 1.f-MATH_CLAMP(r, 0.f, 1.f);
}

static loplayer_combat_attack_t* loplayer_combat_find_first_attack_(
    loplayer_combat_t* combat) {
  assert(combat != NULL);

  loplayer_combat_attack_t* first = NULL;
  for (size_t i = 0; i < LOPLAYER_COMBAT_RESERVE; ++i) {
    loplayer_combat_attack_t* a = &combat->attacks[i];
    if (a->duration > 0) {
      if (first == NULL || first->start > a->start) first = a;
    }
  }
  return first;
}

static loplayer_combat_attack_t* loplayer_combat_find_unused_attack_(
    loplayer_combat_t* combat) {
  assert(combat != NULL);

  for (size_t i = 0; i < LOPLAYER_COMBAT_RESERVE; ++i) {
    if (combat->attacks[i].duration == 0) return &combat->attacks[i];
  }
  return NULL;
}

void loplayer_combat_initialize(
    loplayer_combat_t*       combat,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities) {
  assert(combat   != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);

  *combat = (typeof(*combat)) {
    .ticker   = ticker,
    .entities = entities,
  };
}

void loplayer_combat_deinitialize(loplayer_combat_t* combat) {
  assert(combat != NULL);

  loplayer_combat_drop_all_attack(combat, true  /* = by system */);
}

bool loplayer_combat_add_attack(
    loplayer_combat_t* combat, const loplayer_combat_attack_t* attack) {
  assert(combat != NULL);
  assert(attack != NULL);

  if (!combat->accepting) return false;

  const uint64_t st = attack->start;
  const uint64_t ed = st + attack->duration;
  if (ed <= combat->ticker->time || attack->duration == 0) return false;

  for (size_t i = 0; i < LOPLAYER_COMBAT_RESERVE; ++i) {
    const loplayer_combat_attack_t* a = &combat->attacks[i];
    if (a->duration == 0) continue;

    const uint64_t ist = a->start;
    const uint64_t ied = ist + a->duration;
    if (st < ied && ist < ed) return false;
  }

  loplayer_combat_attack_t* a = loplayer_combat_find_unused_attack_(combat);
  if (a == NULL) return false;

  *a = *attack;
  if (combat->first_attack == NULL ||
      combat->first_attack->start > a->start) {
    combat->first_attack = a;
  }
  if (combat->last_attack == NULL ||
      combat->last_attack->start < a->start) {
    combat->last_attack = a;
  }
  return true;
}

void loplayer_combat_drop_dead_attack(loplayer_combat_t* combat) {
  assert(combat != NULL);

  for (size_t i = 0; i < LOPLAYER_COMBAT_RESERVE; ++i) {
    loplayer_combat_attack_t* a = &combat->attacks[i];
    if (a->duration == 0) continue;

    loentity_store_iterator_t itr = {0};
    if (loentity_store_find_item_by_id(
          combat->entities, &itr, a->attacker)) {
      continue;
    }

    if (combat->first_attack == a) combat->first_attack = NULL;
    if (combat->last_attack  == a) combat->last_attack  = NULL;
    *a = (typeof(*a)) {0};
  }
}

void loplayer_combat_drop_all_attack(
    loplayer_combat_t* combat, loplayer_combat_attack_result_t reason) {
  assert(combat != NULL);

  for (size_t i = 0; i < LOPLAYER_COMBAT_RESERVE; ++i) {
    loplayer_combat_attack_t* a = &combat->attacks[i];
    if (a->duration != 0) loplayer_combat_attack_handle(a, reason);
    *a = (typeof(*a)) {0};
  }
  combat->first_attack = NULL;
  combat->last_attack  = NULL;
}

bool loplayer_combat_pop_attack(
    loplayer_combat_t*        combat,
    loplayer_combat_attack_t* attack,
    float*                    guard_ratio) {
  assert(combat      != NULL);
  assert(attack      != NULL);
  assert(guard_ratio != NULL);

  const uint64_t t  = combat->ticker->time;
  const uint64_t pt = combat->ticker->prev_time;

  loplayer_combat_attack_t* first = combat->first_attack;
  if (first == NULL) return false;

  const uint64_t ed = first->start + first->duration;
  if (pt >= ed || ed > t) return false;

  *attack      = *first;
  *guard_ratio = loplayer_combat_calc_guard_ratio_(combat, first);
  *first       = (typeof(*first)) {0};

  combat->first_attack = loplayer_combat_find_first_attack_(combat);
  if (combat->first_attack == NULL) combat->last_attack = NULL;
  return true;
}

void loplayer_combat_set_accepting(
    loplayer_combat_t*              combat,
    bool                            accepting,
    loplayer_combat_attack_result_t reason) {
  assert(combat != NULL);

  if (combat->accepting == accepting) return;

  combat->accepting = accepting;
  if (!accepting) loplayer_combat_drop_all_attack(combat, reason);
}

void loplayer_combat_set_guarding(loplayer_combat_t* combat, bool guarding) {
  assert(combat != NULL);

  if (combat->last_guard_start <= combat->last_guard_end) {
    if (guarding) combat->last_guard_start = combat->ticker->time;
  } else {
    if (!guarding) combat->last_guard_end = combat->ticker->time;
  }
}

bool loplayer_combat_is_attack_pending(const loplayer_combat_t* combat) {
  assert(combat != NULL);

  if (combat->last_attack == NULL) return false;

  const uint64_t st = combat->last_attack->start;
  const uint64_t ed = st + combat->last_attack->duration;

  return ed > combat->ticker->time;
}

void loplayer_combat_attack_handle(
    const loplayer_combat_attack_t* attack,
    loplayer_combat_attack_result_t result) {
  assert(attack != NULL);

  assert(attack->handle != NULL);
  attack->handle(attack, result);
}
