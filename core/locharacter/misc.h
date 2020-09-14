#pragma once

#include <stdbool.h>
#include <stddef.h>

/* dont forget to update EACH macro */
typedef enum {
  LOCHARACTER_TYPE_ENCEPHALON,
  LOCHARACTER_TYPE_CAVIA,
  LOCHARACTER_TYPE_SCIENTIST,
  LOCHARACTER_TYPE_WARDER,
  LOCHARACTER_TYPE_THEISTS_CHILD,
  LOCHARACTER_TYPE_BIG_WARDER,
  LOCHARACTER_TYPE_GREEDY_SCIENTIST,
} locharacter_type_t;

#define LOCHARACTER_TYPE_EACH_(PROC) do {  \
  PROC(ENCEPHALON,       encephalon);  \
  PROC(CAVIA,            cavia);  \
  PROC(SCIENTIST,        scientist);  \
  PROC(WARDER,           warder);  \
  PROC(THEISTS_CHILD,    theists_child);  \
  PROC(BIG_WARDER,       big_warder);  \
  PROC(GREEDY_SCIENTIST, greedy_scientist);  \
} while (0)

/* dont forget to update EACH macro */
typedef enum {
  LOCHARACTER_STATE_WAIT,
  LOCHARACTER_STATE_STANDUP,
  LOCHARACTER_STATE_WALK,
  LOCHARACTER_STATE_SHOOT,
  LOCHARACTER_STATE_RUSH,
  LOCHARACTER_STATE_THRUST,
  LOCHARACTER_STATE_COMBO,
  LOCHARACTER_STATE_COOLDOWN,
  LOCHARACTER_STATE_STUNNED,
  LOCHARACTER_STATE_DEAD,
} locharacter_state_t;

#define LOCHARACTER_STATE_EACH_(PROC) do {  \
  PROC(WAIT,     wait);  \
  PROC(STANDUP,  standup);  \
  PROC(WALK,     walk);  \
  PROC(SHOOT,    shoot);  \
  PROC(RUSH,     rush);  \
  PROC(THRUST,   thrust);  \
  PROC(COMBO,    combo);  \
  PROC(COOLDOWN, cooldown);  \
  PROC(STUNNED,  stunned);  \
  PROC(DEAD,     dead);  \
} while (0)

const char*
locharacter_type_stringify(
    locharacter_type_t type
);

bool
locharacter_type_unstringify(
    locharacter_type_t* type,
    const char*         v,
    size_t              len
);

const char*
locharacter_state_stringify(
    locharacter_state_t state
);

bool
locharacter_state_unstringify(
    locharacter_state_t* state,
    const char*          v,
    size_t               len
);
