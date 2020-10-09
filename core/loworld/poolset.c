#include "./poolset.h"

#include <assert.h>
#include <stddef.h>

#include <msgpack.h>
#include <msgpack/sbuffer.h>

#include "core/lobullet/pool.h"
#include "core/lochara/pool.h"
#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/store.h"
#include "core/loground/pool.h"
#include "core/loparticle/pool.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#define GROUNDS_PER_CHUNK_    16
#define PARTICLES_PER_CHUNK_  16
#define BULLETS_PER_CHUNK_    64
#define CHARACTERS_PER_CHUNK_ 16

void loworld_poolset_initialize(
    loworld_poolset_t*       pools,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    locommon_counter_t*      idgen,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities,
    loplayer_t*              player,
    size_t                   max_chunks) {
  assert(pools    != NULL);
  assert(res      != NULL);
  assert(shaders  != NULL);
  assert(idgen    != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);
  assert(player   != NULL);
  assert(max_chunks > 0);

  *pools = (typeof(*pools)) {0};

  pools->ground = loground_pool_new(
      &shaders->drawer.ground,
      idgen,
      max_chunks*GROUNDS_PER_CHUNK_);

  pools->particle = loparticle_pool_new(
      &shaders->drawer.particle,
      idgen,
      ticker,
      entities,
      max_chunks*PARTICLES_PER_CHUNK_);

  pools->bullet = lobullet_pool_new(
      res,
      shaders,
      idgen,
      ticker,
      entities,
      max_chunks*BULLETS_PER_CHUNK_);

  pools->chara = lochara_pool_new(
      res,
      shaders,
      idgen,
      ticker,
      entities,
      player,
      pools->bullet,
      max_chunks*CHARACTERS_PER_CHUNK_);
}

void loworld_poolset_deinitialize(loworld_poolset_t* pools) {
  assert(pools != NULL);

  lochara_pool_delete(pools->chara);
  lobullet_pool_delete(pools->bullet);
  loparticle_pool_delete(pools->particle);
  loground_pool_delete(pools->ground);
}

loentity_t* loworld_poolset_unpack_entity(
    const loworld_poolset_t* pools, const msgpack_object* obj) {
  assert(pools != NULL);
  assert(obj   != NULL);

  loentity_t* e;

  e = (typeof(e)) loground_pool_unpack_item(pools->ground, obj);
  if (e != NULL) return e;

  e = (typeof(e)) loparticle_pool_unpack_item(pools->particle, obj);
  if (e != NULL) return e;

  e = (typeof(e)) lobullet_pool_unpack_item(pools->bullet, obj);
  if (e != NULL) return e;

  e = (typeof(e)) lochara_pool_unpack_item(pools->chara, obj);
  if (e != NULL) return e;

  return NULL;
}
