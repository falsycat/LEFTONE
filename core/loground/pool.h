#pragma once

#include <stddef.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/lobullet/pool.h"
#include "core/locommon/counter.h"
#include "core/loshader/ground.h"

#include "./base.h"

struct loground_pool_t;
typedef struct loground_pool_t loground_pool_t;

loground_pool_t*  /* OWNERSHIP */
loground_pool_new(
    loshader_ground_drawer_t* drawer,
    locommon_counter_t*       idgen,
    size_t                    length
);

void
loground_pool_delete(
    loground_pool_t* pool  /* OWNERSHIP */
);

loground_base_t*  /* OWNERSHIP */
loground_pool_create(
    loground_pool_t* pool
);

loground_base_t*  /* OWNERSHIP/NULLABLE */
loground_pool_unpack_item(
    loground_pool_t*      pool,
    const msgpack_object* obj
);
