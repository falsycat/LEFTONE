#include "./pool.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <msgpack.h>

#include "util/memory/memory.h"

#include "core/locommon/counter.h"
#include "core/loentity/pool.h"
#include "core/loshader/ground.h"

#include "./base.h"

LOENTITY_POOL_SOURCE_TEMPLATE(loground)

loground_pool_t* loground_pool_new(
    loshader_ground_drawer_t* drawer,
    locommon_counter_t*       idgen,
    size_t                    length) {
  assert(drawer != NULL);
  assert(idgen  != NULL);
  assert(length > 0);

  loground_pool_t* pool = memory_new(
      sizeof(*pool) + (length-1)*sizeof(pool->items[0]));
  *pool = (typeof(*pool)) {
    .idgen  = idgen,
    .length = length,
  };

  for (size_t i = 0; i < pool->length; ++i) {
    loground_base_initialize(&pool->items[i], drawer);
  }
  return pool;
}
