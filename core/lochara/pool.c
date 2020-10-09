#include "./pool.h"

#include <assert.h>
#include <stddef.h>

#include <msgpack.h>

#include "util/memory/memory.h"

#include "core/lobullet/pool.h"
#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/pool.h"
#include "core/loentity/entity.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./base.h"

LOENTITY_POOL_SOURCE_TEMPLATE(lochara)

lochara_pool_t* lochara_pool_new(
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    locommon_counter_t*      idgen,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities,
    loplayer_t*              player,
    lobullet_pool_t*         bullet,
    size_t                   length) {
  assert(res      != NULL);
  assert(shaders  != NULL);
  assert(idgen    != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);
  assert(player   != NULL);
  assert(bullet   != NULL);
  assert(length > 0);

  lochara_pool_t* pool =
      memory_new(sizeof(*pool) + (length-1)*sizeof(pool->items[0]));
  *pool = (typeof(*pool)) {
    .idgen  = idgen,
    .length = length,
  };
  for (size_t i = 0; i < pool->length; ++i) {
    lochara_base_initialize(
        &pool->items[i],
        res,
        shaders,
        ticker,
        entities,
        player,
        bullet);
  }
  return pool;
}
