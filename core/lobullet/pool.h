#pragma once

#include <stddef.h>

#include <msgpack.h>

#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/store.h"
#include "core/loresource/set.h"
#include "core/loshader/bullet.h"

#include "./base.h"

struct lobullet_pool_t;
typedef struct lobullet_pool_t lobullet_pool_t;

lobullet_pool_t*  /* OWNERSHIP */
lobullet_pool_new(
    loresource_set_t*         res,
    loshader_bullet_drawer_t* drawer,
    locommon_counter_t*       idgen,
    const locommon_ticker_t*  ticker,
    loentity_store_t*         entities,
    size_t                    length
);

void
lobullet_pool_delete(
    lobullet_pool_t* pool  /* OWNERSHIP */
);

lobullet_base_t*
lobullet_pool_create(
    lobullet_pool_t* pool
);

lobullet_base_t*
lobullet_pool_unpack_item(
    lobullet_pool_t*      pool,
    const msgpack_object* obj
);
