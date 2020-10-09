#pragma once

#include "util/statman/statman.h"

#include "./combat.h"

typedef enum {
  /* BENUM BEGIN lochara_strategy */
  LOCHARA_STRATEGY_WAIT,
  LOCHARA_STRATEGY_WAKE_UP,
  LOCHARA_STRATEGY_WAKE_UP_EVENT,

  LOCHARA_STRATEGY_SCOUTING,
  LOCHARA_STRATEGY_APPROACH,

  LOCHARA_STRATEGY_COMBO1,
  LOCHARA_STRATEGY_COMBO2,
  LOCHARA_STRATEGY_COMBO3,

  LOCHARA_STRATEGY_SHOOT1,
  LOCHARA_STRATEGY_SHOOT2,
  LOCHARA_STRATEGY_SHOOT3,

  LOCHARA_STRATEGY_SPELL1,
  LOCHARA_STRATEGY_SPELL2,
  LOCHARA_STRATEGY_SPELL3,

  LOCHARA_STRATEGY_DOWN,
  LOCHARA_STRATEGY_DEAD,
  LOCHARA_STRATEGY_KILL,
  /* BENUM END */
} lochara_strategy_t;

/* generated benum header */
#include "core/lochara/benum/strategy.h"


/* ---- default strategy handlers ---- */

void
lochara_strategy_initialize_any_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);

void
lochara_strategy_cancel_transition_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);

void
lochara_strategy_initialize_scouting_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);

void
lochara_strategy_update_scouting_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);

void
lochara_strategy_initialize_combat_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);

void
lochara_strategy_update_combat_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);


/* ---- default strategy constructors ---- */

typedef struct {
  const statman_meta_t* state_table;

  uint64_t period;
  float    stagger;  /* 0~1 */
  float    range_back;
  float    range_close;
  float    range_mid;
  float    range_long;

  lochara_strategy_t found_close;
  lochara_strategy_t found_mid;
  lochara_strategy_t found_long;
} lochara_strategy_scouting_param_t;
#define lochara_strategy_scouting(...)  \
    (statman_meta_t) {  \
      .state      = LOCHARA_STRATEGY_SCOUTING,  \
      .name       = "SCOUTING",  \
      .data       = &(lochara_strategy_scouting_param_t) {  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_strategy_initialize_scouting_,  \
      .update     = lochara_strategy_update_scouting_,  \
    }

typedef struct {
  const statman_meta_t*          state_table;
  const lochara_combat_action_t* actions;

  uint64_t           parry_window;
  lochara_strategy_t parried_next;

  bool gravity;

  lochara_strategy_t next;
} lochara_strategy_combat_param_t;
#define lochara_strategy_combat(NAME, ...)  \
    (statman_meta_t) {  \
      .state      = LOCHARA_STRATEGY_##NAME,  \
      .name       = #NAME,  \
      .data  = &(lochara_strategy_combat_param_t) {  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_strategy_initialize_combat_,  \
      .update     = lochara_strategy_update_combat_,  \
      .finalize   = lochara_strategy_cancel_transition_,  \
    }
