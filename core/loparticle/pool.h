#pragma once

#include <stddef.h>

#include <msgpack.h>

#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/store.h"
#include "core/loshader/particle.h"

#include "./base.h"

struct loparticle_pool_t;
typedef struct loparticle_pool_t loparticle_pool_t;

loparticle_pool_t*  /* OWNERSHIP */
loparticle_pool_new(
    loshader_particle_drawer_t* drawer,
    locommon_counter_t*         idgen,
    const locommon_ticker_t*    ticker,
    loentity_store_t*           entities,
    size_t                      length
);

void
loparticle_pool_delete(
    loparticle_pool_t* pool  /* OWNERSHIP */
);

loparticle_base_t*
loparticle_pool_create(
    loparticle_pool_t* pool
);

loparticle_base_t*
loparticle_pool_unpack_item(
    loparticle_pool_t*    pool,
    const msgpack_object* obj
);
