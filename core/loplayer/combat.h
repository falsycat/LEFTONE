#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/locommon/ticker.h"
#include "core/loeffect/effect.h"
#include "core/loentity/store.h"
#include "core/loresource/sound.h"
#include "core/loshader/combat_ring.h"

#include "./entity.h"
#include "./status.h"

struct loplayer_combat_t;
typedef struct loplayer_combat_t loplayer_combat_t;

typedef struct {
  loentity_id_t attacker;

  uint64_t start;
  uint64_t duration;

  vec2_t knockback;

  loeffect_t effect;
} loplayer_combat_attack_t;

loplayer_combat_t*  /* OWNERSHIP */
loplayer_combat_new(
    loresource_sound_t*            sound,
    loshader_combat_ring_drawer_t* drawer,
    const locommon_ticker_t*       ticker,
    loentity_store_t*              entities,
    loplayer_status_t*             status,
    loplayer_entity_t*             entity,
    size_t                         length
);

void
loplayer_combat_delete(
    loplayer_combat_t* combat  /* OWNERSHIP */
);

bool
loplayer_combat_add_attack(
    loplayer_combat_t*              combat,
    const loplayer_combat_attack_t* attack
);

bool
loplayer_combat_accept_all_attacks(
    loplayer_combat_t* combat
);

void
loplayer_combat_drop_all_attacks(
    loplayer_combat_t* combat
);

void
loplayer_combat_guard(
    loplayer_combat_t* combat
);

void
loplayer_combat_unguard(
    loplayer_combat_t* combat
);

void
loplayer_combat_update(
    loplayer_combat_t* combat
);

void
loplayer_combat_draw_ui(
    const loplayer_combat_t* combat
);

void
loplayer_combat_pack(
    const loplayer_combat_t* combat,
    msgpack_packer*          packer
);

bool
loplayer_combat_unpack(
    loplayer_combat_t*    combat,
    const msgpack_object* obj  /* NULLABLE */
);

void
loplayer_combat_attack_pack(
    const loplayer_combat_attack_t* attack,
    msgpack_packer*                 packer
);

bool
loplayer_combat_attack_unpack(
    loplayer_combat_attack_t* attack,
    const msgpack_object*     obj  /* NULLABLE */
);
