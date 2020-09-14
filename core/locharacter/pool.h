#pragma once

#include <stddef.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/lobullet/pool.h"
#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/store.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/character.h"

#include "./base.h"

struct locharacter_pool_t;
typedef struct locharacter_pool_t locharacter_pool_t;

locharacter_pool_t*  /* OWNERSHIP */
locharacter_pool_new(
    loresource_set_t*            res,
    loshader_character_drawer_t* drawer,
    locommon_counter_t*          idgen,
    const locommon_ticker_t*     ticker,
    lobullet_pool_t*             bullets,
    loentity_store_t*            entities,
    loplayer_t*                  player,
    size_t                       length
);

void
locharacter_pool_delete(
    locharacter_pool_t* pool  /* OWNERSHIP */
);

locharacter_base_t*
locharacter_pool_create(
    locharacter_pool_t* pool
);

locharacter_base_t*
locharacter_pool_unpack_item(
    locharacter_pool_t*   pool,
    const msgpack_object* obj
);
