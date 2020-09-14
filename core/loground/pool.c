#include "./pool.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <msgpack.h>

#include "util/memory/memory.h"

#include "core/locommon/counter.h"
#include "core/loshader/ground.h"

#include "./base.h"

struct loground_pool_t {
  loshader_ground_drawer_t* drawer;
  locommon_counter_t*       idgen;

  size_t length;
  loground_base_t items[1];
};

static size_t loground_pool_find_unused_item_index_(
    const loground_pool_t* pool) {
  assert(pool != NULL);

  for (size_t i = 0; i < pool->length; ++i) {
    if (!pool->items[i].used) return i;
  }
  fprintf(stderr, "ground pool overflow\n");
  abort();
}

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
    .drawer = drawer,
    .idgen  = idgen,
    .length = length,
  };

  for (size_t i = 0; i < pool->length; ++i) {
    loground_base_initialize(&pool->items[i], drawer);
  }
  return pool;
}

void loground_pool_delete(loground_pool_t* pool) {
  assert(pool != NULL);

  for (size_t i = 0; i < pool->length; ++i) {
    loground_base_deinitialize(&pool->items[i]);
  }
  memory_delete(pool);
}

loground_base_t* loground_pool_create(loground_pool_t* pool) {
  assert(pool != NULL);

  const size_t i = loground_pool_find_unused_item_index_(pool);

  loground_base_reinitialize(
      &pool->items[i], locommon_counter_count(pool->idgen));

  pool->items[i].used = true;
  return &pool->items[i];
}

loground_base_t* loground_pool_unpack_item(
    loground_pool_t* pool, const msgpack_object* obj) {
  assert(pool != NULL);

  const size_t i = loground_pool_find_unused_item_index_(pool);

  if (!loground_base_unpack(&pool->items[i], obj)) return NULL;

  pool->items[i].used = true;
  return &pool->items[i];
}
