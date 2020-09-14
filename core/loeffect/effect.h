#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "core/locommon/null.h"

#include "./generic.h"

typedef enum {
  LOEFFECT_ID_IMMEDIATE_DAMAGE,

  LOEFFECT_ID_CURSE,
      /* The curse effect actually does nothing and is just for HUD.
       * To kill player immediately, use curse trigger effect.*/
  LOEFFECT_ID_CURSE_TRIGGER,

  LOEFFECT_ID_AMNESIA,
  LOEFFECT_ID_LOST,
} loeffect_id_t;

typedef struct {
  loeffect_id_t id;
  union {
    locommon_null_t                    null;
    loeffect_generic_immediate_param_t imm;
    loeffect_generic_lasting_param_t   lasting;
  } data;
} loeffect_t;

#define loeffect_immediate_damage(a)  \
    ((loeffect_t) {  \
      .id = LOEFFECT_ID_IMMEDIATE_DAMAGE,  \
      .data = { .imm = {  \
        .amount = a,  \
      }, },  \
    } )

#define loeffect_curse(b, dur)  \
    ((loeffect_t) {  \
      .id = LOEFFECT_ID_CURSE,  \
      .data = { .lasting = {  \
        .begin    = b,  \
        .duration = dur,  \
      }, },  \
    } )

#define loeffect_curse_trigger()  \
    ((loeffect_t) {  \
      .id = LOEFFECT_ID_CURSE_TRIGGER,  \
    } )

#define loeffect_amnesia(b, dur)  \
    ((loeffect_t) {  \
      .id = LOEFFECT_ID_AMNESIA,  \
      .data = { .lasting = {  \
        .begin    = b,  \
        .duration = dur,  \
      }, },  \
    } )

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
