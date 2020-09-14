#include "./pool.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <msgpack.h>

#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "core/lobullet/pool.h"
#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/store.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/character.h"

#include "./base.h"

struct locharacter_pool_t {
  loresource_set_t*            res;
  loshader_character_drawer_t* drawer;
  locommon_counter_t*          idgen;
  const locommon_ticker_t*     ticker;
  lobullet_pool_t*             bullets;
  loentity_store_t*            entities;
  loplayer_t*                  player;

  size_t length;
  locharacter_base_t items[1];
};

static size_t locharacter_pool_find_unused_item_index_(
    const locharacter_pool_t* pool) {
  assert(pool != NULL);

  for (size_t i = 0; i < pool->length; ++i) {
    if (!pool->items[i].used) return i;
  }
  fprintf(stderr, "character pool overflow\n");
  abort();
}

locharacter_pool_t* locharacter_pool_new(
    loresource_set_t*            res,
    loshader_character_drawer_t* drawer,
    locommon_counter_t*          idgen,
    const locommon_ticker_t*     ticker,
    lobullet_pool_t*             bullets,
    loentity_store_t*            entities,
    loplayer_t*                  player,
    size_t                       length) {
  assert(res      != NULL);
  assert(drawer   != NULL);
  assert(idgen    != NULL);
  assert(ticker   != NULL);
  assert(bullets  != NULL);
  assert(entities != NULL);
  assert(player   != NULL);
  assert(length > 0);

  locharacter_pool_t* pool = memory_new(
      sizeof(*pool) + (length-1)*sizeof(pool->items[0]));
  *pool = (typeof(*pool)) {
    .res      = res,
    .drawer   = drawer,
    .idgen    = idgen,
    .ticker   = ticker,
    .bullets  = bullets,
    .entities = entities,
    .player   = player,
    .length   = length,
  };

  for (size_t i = 0; i < pool->length; ++i) {
    locharacter_base_initialize(
        &pool->items[i],
        res,
        drawer,
        ticker,
        bullets,
        entities,
        player);
  }
  return pool;
}

void locharacter_pool_delete(locharacter_pool_t* pool) {
  assert(pool != NULL);

  for (size_t i = 0; i < pool->length; ++i) {
    locharacter_base_deinitialize(&pool->items[i]);
  }
  memory_delete(pool);
}

locharacter_base_t* locharacter_pool_create(locharacter_pool_t* pool) {
  assert(pool != NULL);

  const size_t i = locharacter_pool_find_unused_item_index_(pool);

  locharacter_base_reinitialize(
      &pool->items[i], locommon_counter_count(pool->idgen));

  pool->items[i].used = true;
  return &pool->items[i];
}

locharacter_base_t* locharacter_pool_unpack_item(
    locharacter_pool_t* pool, const msgpack_object* obj) {
  assert(pool != NULL);

  const size_t i = locharacter_pool_find_unused_item_index_(pool);

  if (!locharacter_base_unpack(&pool->items[i], obj)) return NULL;

  pool->items[i].used = true;
  return &pool->items[i];
}
