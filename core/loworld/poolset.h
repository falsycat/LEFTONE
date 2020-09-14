#pragma once

#include <stddef.h>

#include <msgpack.h>

#include "core/lobullet/pool.h"
#include "core/locharacter/pool.h"
#include "core/loground/pool.h"

typedef struct {
  loground_pool_t*    ground;
  lobullet_pool_t*    bullet;
  locharacter_pool_t* character;
} loworld_poolset_t;

/* Initialize and Deinitialize each member manually
 * because of a dependency issue. */

loentity_t*  /* NULLABLE/OWNERSHIP */
loworld_poolset_unpack_entity(
    const loworld_poolset_t* pools,
    const msgpack_object*    obj
);

void
loworld_poolset_test_packing(
    const loworld_poolset_t* pools
);
