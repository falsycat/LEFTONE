#pragma once

#include <stddef.h>

#include <msgpack.h>

#include "core/lobullet/pool.h"
#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./base.h"

struct lochara_pool_t;
typedef struct lochara_pool_t lochara_pool_t;

lochara_pool_t*  /* OWNERSHIP */
lochara_pool_new(
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    locommon_counter_t*      idgen,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities,
    loplayer_t*              player,
    lobullet_pool_t*         bullet,
    size_t                   length
);

void
lochara_pool_delete(
    lochara_pool_t* pool  /* OWNERSHIP */
);

lochara_base_t*
lochara_pool_create(
    lochara_pool_t* pool
);

lochara_base_t*
lochara_pool_unpack_item(
    lochara_pool_t*       pool,
    const msgpack_object* obj
);
