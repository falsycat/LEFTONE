#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/recipient.h"
#include "core/loeffect/stance.h"
#include "core/loresource/set.h"

typedef struct {
  loresource_set_t*        res;
  const locommon_ticker_t* ticker;

  loeffect_stance_set_t stances;
  loeffect_recipient_t  recipient;

  locommon_position_t respawn_pos;

  uint64_t last_damage_time;
  uint64_t bullet_immune_until;

  float dying_effect;  /* 0~1 */
} loplayer_status_t;

void
loplayer_status_initialize(
    loplayer_status_t*       status,
    loresource_set_t*        res,
    const locommon_ticker_t* ticker
);

void
loplayer_status_deinitialize(
    loplayer_status_t* status
);

void
loplayer_status_reset(
    loplayer_status_t* status
);

bool
loplayer_status_add_stance(
    loplayer_status_t*   status,
    loeffect_stance_id_t id
);

void
loplayer_status_remove_stance(
    loplayer_status_t*   status,
    loeffect_stance_id_t id
);

void
loplayer_status_apply_effect(
    loplayer_status_t* status,
    const loeffect_t*  effect
);

void
loplayer_status_update(
    loplayer_status_t* status
);

void
loplayer_status_set_respawn_position(
    loplayer_status_t*         status,
    const locommon_position_t* pos
);

void
loplayer_status_pack(
    const loplayer_status_t* status,
    msgpack_packer*          packer
);

bool
loplayer_status_unpack(
    loplayer_status_t*    status,
    const msgpack_object* obj
);
