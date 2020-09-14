#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/locommon/ticker.h"

#include "./effect.h"
#include "./generic.h"

typedef struct {
  float attack;
  float defence;

  float speed;  /* [chunks/sec] */
  float jump;   /* [chunks/sec^2] */
} loeffect_recipient_status_t;

typedef struct {
  loeffect_generic_lasting_param_t curse;
  loeffect_generic_lasting_param_t amnesia;
} loeffect_recipient_effect_param_t;

typedef struct {
  const locommon_ticker_t* ticker;

  float         madness;
  float         faith;
  loeffect_id_t last_damage;

  loeffect_recipient_effect_param_t effects;

  loeffect_recipient_status_t status;

} loeffect_recipient_t;

void
loeffect_recipient_initialize(
    loeffect_recipient_t*    recipient,
    const locommon_ticker_t* ticker
);

void
loeffect_recipient_deinitialize(
    loeffect_recipient_t* recipient
);

void
loeffect_recipient_reset(
    loeffect_recipient_t* recipient
);

void
loeffect_recipient_apply_effect(
    loeffect_recipient_t* recipient,
    const loeffect_t*     effect
);

void
loeffect_recipient_update(
    loeffect_recipient_t*              recipient,
    const loeffect_recipient_status_t* base
);

void
loeffect_recipient_effect_param_pack(
    const loeffect_recipient_effect_param_t* recipient,
    msgpack_packer*                          packer
);

bool
loeffect_recipient_effect_param_unpack(
    loeffect_recipient_effect_param_t* recipient,
    const msgpack_object*              obj  /* NULLABLE */
);
