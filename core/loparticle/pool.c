#include "./pool.h"

#include <stddef.h>

#include <msgpack.h>

#include "util/memory/memory.h"

#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/pool.h"
#include "core/loentity/store.h"
#include "core/loshader/particle.h"

#include "./base.h"

LOENTITY_POOL_SOURCE_TEMPLATE(loparticle)

loparticle_pool_t* loparticle_pool_new(
    loshader_particle_drawer_t* drawer,
    locommon_counter_t*         idgen,
    const locommon_ticker_t*    ticker,
    loentity_store_t*           entities,
    size_t                      length) {
  assert(drawer   != NULL);
  assert(idgen    != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);
  assert(length > 0);

  loparticle_pool_t* pool =
      memory_new(sizeof(*pool) + (length-1)*sizeof(pool->items[0]));
  *pool = (typeof(*pool)) {
    .idgen  = idgen,
    .length = length,
  };
  for (size_t i = 0; i < pool->length; ++i) {
    loparticle_base_initialize(
        &pool->items[i],
        drawer,
        ticker,
        entities);
  }
  return pool;
}
