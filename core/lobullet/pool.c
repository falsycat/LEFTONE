#include "./pool.h"

#include <stddef.h>

#include <msgpack.h>

#include "util/memory/memory.h"

#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/pool.h"
#include "core/loentity/store.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./base.h"

LOENTITY_POOL_SOURCE_TEMPLATE(lobullet)

lobullet_pool_t* lobullet_pool_new(
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    locommon_counter_t*      idgen,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities,
    size_t                   length) {
  assert(res      != NULL);
  assert(shaders  != NULL);
  assert(idgen    != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);
  assert(length > 0);

  lobullet_pool_t* pool =
      memory_new(sizeof(*pool) + (length-1)*sizeof(pool->items[0]));
  *pool = (typeof(*pool)) {
    .idgen  = idgen,
    .length = length,
  };
  for (size_t i = 0; i < pool->length; ++i) {
    lobullet_base_initialize(
        &pool->items[i],
        res,
        shaders,
        ticker,
        entities);
  }
  return pool;
}
