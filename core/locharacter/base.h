#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/vector.h"

#include "core/lobullet/pool.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/recipient.h"
#include "core/loentity/character.h"
#include "core/loentity/store.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/character.h"

#include "./misc.h"

typedef struct {
  loentity_character_t super;
  bool                 used;

  /* injected deps */
  loresource_set_t*            res;
  loshader_character_drawer_t* drawer;
  const locommon_ticker_t*     ticker;
  lobullet_pool_t*             bullets;
  loentity_store_t*            entities;
  loplayer_t*                  player;

  /* temporary params for update */
  struct {
    /* set before calling update function */
    loentity_ground_t* ground;
    vec2_t             player_pos;

    uint64_t time;
        /* Defaultly equals to ticker->time.
           But characters who have an event with music
           overwrites this value for synchronization */

    /* set by update function */
    float height;
    bool  bullet_hittest;
    bool  gravity;

    loshader_character_drawer_instance_t instance;
  } cache;

  /* params to be packed (includes id) */
  locharacter_type_t type;

  loentity_id_t ground;
  vec2_t        pos;
  float         direction;
  vec2_t        knockback;
  float         gravity;

  loeffect_recipient_t recipient;

  locharacter_state_t state;
  uint64_t            since;

  uint64_t last_update_time;
  uint64_t last_knockback_time;
  uint64_t last_hit_time;

# define LOCHARACTER_BASE_DATA_MAX_SIZE 256
  uint8_t data[LOCHARACTER_BASE_DATA_MAX_SIZE];
} locharacter_base_t;

void
locharacter_base_initialize(
    locharacter_base_t*          base,
    loresource_set_t*            res,
    loshader_character_drawer_t* drawer,
    const locommon_ticker_t*     ticker,
    lobullet_pool_t*             bullets,
    loentity_store_t*            entities,
    loplayer_t*                  player
);

void
locharacter_base_reinitialize(
    locharacter_base_t* base,
    loentity_id_t       id
);

void
locharacter_base_deinitialize(
    locharacter_base_t* base
);

bool
locharacter_base_unpack(
    locharacter_base_t*   base,
    const msgpack_object* obj
);
