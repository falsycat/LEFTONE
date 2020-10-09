#pragma once

#include <stddef.h>

#include <msgpack.h>

#include "core/lobullet/pool.h"
#include "core/lochara/pool.h"
#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/store.h"
#include "core/loground/pool.h"
#include "core/loparticle/pool.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

typedef struct {
  loground_pool_t*    ground;
  loparticle_pool_t*  particle;
  lobullet_pool_t*    bullet;
  lochara_pool_t*     chara;
} loworld_poolset_t;

void
loworld_poolset_initialize(
    loworld_poolset_t*       pools,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    locommon_counter_t*      idgen,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities,
    loplayer_t*              player,
    size_t                   max_chunks
);

void
loworld_poolset_deinitialize(
    loworld_poolset_t* pools
);

loentity_t*  /* NULLABLE/OWNERSHIP */
loworld_poolset_unpack_entity(
    const loworld_poolset_t* pools,
    const msgpack_object*    obj
);
