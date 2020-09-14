#include "./poolset.h"

#include <assert.h>
#include <stddef.h>

#include <msgpack.h>
#include <msgpack/sbuffer.h>

#include "core/lobullet/pool.h"
#include "core/locharacter/pool.h"
#include "core/loground/pool.h"

#include "./test.h"

loentity_t* loworld_poolset_unpack_entity(
    const loworld_poolset_t* pools, const msgpack_object* obj) {
  assert(pools != NULL);
  assert(obj   != NULL);

  loentity_t* e;

  e = (typeof(e)) loground_pool_unpack_item(pools->ground, obj);
  if (e != NULL) return e;

  e = (typeof(e)) lobullet_pool_unpack_item(pools->bullet, obj);
  if (e != NULL) return e;

  e = (typeof(e)) locharacter_pool_unpack_item(pools->character, obj);
  if (e != NULL) return e;

  return NULL;
}

void loworld_poolset_test_packing(const loworld_poolset_t* pools) {
  assert(pools != NULL);

  loworld_test_packing(pools);
}
