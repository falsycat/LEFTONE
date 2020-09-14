#include "./pool.h"

#include <stddef.h>

#include <msgpack.h>

#include "util/memory/memory.h"

#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/store.h"
#include "core/loresource/set.h"
#include "core/loshader/bullet.h"

#include "./base.h"

struct lobullet_pool_t {
  loresource_set_t*         res;
  loshader_bullet_drawer_t* drawer;
  locommon_counter_t*       idgen;
  const locommon_ticker_t*  ticker;
  loentity_store_t*         entities;

  size_t length;
  lobullet_base_t items[1];
};

static size_t lobullet_pool_find_unused_item_index_(
    const lobullet_pool_t* pool) {
  assert(pool != NULL);

  for (size_t i = 0; i < pool->length; ++i) {
    if (!pool->items[i].used) return i;
  }
  fprintf(stderr, "bullet pool overflow\n");
  abort();
}

lobullet_pool_t* lobullet_pool_new(
    loresource_set_t*         res,
    loshader_bullet_drawer_t* drawer,
    locommon_counter_t*       idgen,
    const locommon_ticker_t*  ticker,
    loentity_store_t*         entities,
    size_t                    length) {
  assert(res      != NULL);
  assert(drawer   != NULL);
  assert(idgen    != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);
  assert(length > 0);

  lobullet_pool_t* pool =
      memory_new(sizeof(*pool) + (length-1)*sizeof(pool->items[0]));
  *pool = (typeof(*pool)) {
    .res      = res,
    .drawer   = drawer,
    .idgen    = idgen,
    .ticker   = ticker,
    .entities = entities,
    .length   = length,
  };
  for (size_t i = 0; i < pool->length; ++i) {
    lobullet_base_initialize(
        &pool->items[i],
        res,
        drawer,
        ticker,
        entities);
  }
  return pool;
}

void lobullet_pool_delete(lobullet_pool_t* pool) {
  if (pool == NULL) return;

  for (size_t i = 0; i < pool->length; ++i) {
    lobullet_base_deinitialize(&pool->items[i]);
  }
  memory_delete(pool);
}

lobullet_base_t* lobullet_pool_create(lobullet_pool_t* pool) {
  assert(pool != NULL);

  const size_t i = lobullet_pool_find_unused_item_index_(pool);

  pool->items[i].used = true;
  lobullet_base_reinitialize(
      &pool->items[i], locommon_counter_count(pool->idgen));
  return &pool->items[i];
}

lobullet_base_t* lobullet_pool_unpack_item(
    lobullet_pool_t* pool, const msgpack_object* obj) {
  assert(pool != NULL);

  const size_t i = lobullet_pool_find_unused_item_index_(pool);

  if (!lobullet_base_unpack(&pool->items[i], obj)) return NULL;

  pool->items[i].used = true;
  return &pool->items[i];
}
