#pragma once

#include <stdbool.h>
#include <stdint.h>

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
  const locommon_ticker_t* ticker;

  loeffect_recipient_status_t status;

  float madness;
  float faith;

  uint64_t      last_die;
  loeffect_id_t last_die_reason;

  uint64_t lost_damage_since;

  uint64_t last_resuscitate;
  uint64_t last_damage;
  uint64_t last_heal;
  uint64_t last_lost;
  uint64_t last_retrieval;

  struct {
    loeffect_generic_lasting_param_t fanatic;
    loeffect_generic_lasting_param_t curse;
    loeffect_generic_lasting_param_t amnesia;
  } effects;
} loeffect_recipient_t;

void
loeffect_recipient_initialize(
    loeffect_recipient_t*              recipient,
    const locommon_ticker_t*           ticker,
    const loeffect_recipient_status_t* status  /* NULLABLE */
);

void
loeffect_recipient_deinitialize(
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

bool
loeffect_recipient_is_alive(
    const loeffect_recipient_t* recipient
);

void
loeffect_recipient_pack(
    const loeffect_recipient_t* recipient,
    msgpack_packer*             packer
);

bool
loeffect_recipient_unpack(
    loeffect_recipient_t* recipient,
    const msgpack_object* obj
);
