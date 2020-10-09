#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "core/locommon/null.h"

#include "./generic.h"

typedef enum {
  /* system effect */
  LOEFFECT_ID_NONE,
  LOEFFECT_ID_RESUSCITATE,
  LOEFFECT_ID_LOST_DAMAGE,
  LOEFFECT_ID_CURSE_TRIGGER,

  LOEFFECT_ID_DAMAGE,
  LOEFFECT_ID_HEAL,
  LOEFFECT_ID_LOST,
  LOEFFECT_ID_RETRIEVAL,

  LOEFFECT_ID_FANATIC,
  LOEFFECT_ID_CURSE,
      /* The curse effect actually does nothing and is just for HUD.
       * To kill player immediately, use curse trigger effect.*/
  LOEFFECT_ID_AMNESIA,
} loeffect_id_t;

#define LOEFFECT_ID_EACH_(PROC) do {  \
  PROC(NONE,          "none",          null);     \
  PROC(RESUSCITATE,   "resuscitate",   null);     \
  PROC(LOST_DAMAGE,   "lost-damage",   null);     \
  PROC(CURSE_TRIGGER, "curse-trigger", null);     \
  PROC(DAMAGE,        "damage",        imm);      \
  PROC(HEAL,          "heal",          imm);      \
  PROC(LOST,          "lost",          imm);      \
  PROC(RETRIEVAL,     "retrieval",     imm);      \
  PROC(FANATIC,       "fanatic",       lasting);  \
  PROC(CURSE,         "curse",         lasting);  \
  PROC(AMNESIA,       "amnesia",       lasting);  \
} while (0)

typedef struct {
  loeffect_id_t id;
  union {
    locommon_null_t                    null;
    loeffect_generic_immediate_param_t imm;
    loeffect_generic_lasting_param_t   lasting;
  } data;
} loeffect_t;

#define loeffect_with_null_data_(ID)  \
    ((loeffect_t) {  \
      .id = LOEFFECT_ID_##ID,  \
    } )
#define loeffect_with_imm_data_(ID, a)  \
    ((loeffect_t) {  \
      .id = LOEFFECT_ID_##ID,  \
      .data = { .imm = {  \
        .amount = a,  \
      }, },  \
    } )
#define loeffect_with_lasting_data_(ID, d)  \
    ((loeffect_t) {  \
      .id = LOEFFECT_ID_##ID,  \
      .data = { .lasting = {  \
        .duration = d,  \
      }, },  \
    } )

#define loeffect_none()          loeffect_with_null_data_(NONE)
#define loeffect_resuscitate()   loeffect_with_null_data_(RESUSCITATE)
#define loeffect_lost_damage()   loeffect_with_null_data_(LOST_DAMAGE)
#define loeffect_curse_trigger() loeffect_with_null_data_(CURSE_TRIGGER)

#define loeffect_damage(a)    loeffect_with_imm_data_(DAMAGE,    a)
#define loeffect_heal(a)      loeffect_with_imm_data_(HEAL,      a)
#define loeffect_lost(a)      loeffect_with_imm_data_(LOST,      a)
#define loeffect_retrieval(a) loeffect_with_imm_data_(RETRIEVAL, a)

#define loeffect_curse(d)   loeffect_with_lasting_data_(CURSE,   d)
#define loeffect_fanatic(d) loeffect_with_lasting_data_(FANATIC, d)
#define loeffect_amnesia(d) loeffect_with_lasting_data_(AMNESIA, d)

const char*
loeffect_id_stringify(
    loeffect_id_t id
);

bool
loeffect_id_unstringify(
    loeffect_id_t* id,
    const char*    str,
    size_t         len
);

void
loeffect_pack(
    const loeffect_t* effect,
    msgpack_packer*   packer
);

bool
loeffect_unpack(
    loeffect_t*           effect,
    const msgpack_object* obj
);
