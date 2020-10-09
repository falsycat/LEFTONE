#include "./base.h"

#include <assert.h>
#include <stdbool.h>

#include <msgpack.h>

#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/msgpack.h"
#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"
#include "core/loshader/particle.h"

#include "./aura.h"
#include "./misc.h"

/* generated serializer */
#include "core/loparticle/crial/base.h"

static void loparticle_base_delete_(loentity_t* entity) {
  assert(entity != NULL);

  loparticle_base_t* base = (typeof(base)) entity;
  if (!base->used) return;

  base->used = false;

# define each_(NAME, name) do {  \
    if (base->type == LOPARTICLE_TYPE_##NAME) {  \
      loparticle_##name##_tear_down(base);  \
      return;  \
    }  \
  } while (0)

  LOPARTICLE_TYPE_EACH_(each_);
  assert(false);

# undef each_
}

static void loparticle_base_die_(loentity_t* entity) {
  assert(entity != NULL);
}

static bool loparticle_base_update_(loentity_t* entity) {
  assert(entity != NULL);

  loparticle_base_t* base = (typeof(base)) entity;
  base->cache = (typeof(base->cache)) {0};

# define each_(NAME, name) do {  \
    if (base->type == LOPARTICLE_TYPE_##NAME) {  \
      return loparticle_##name##_update(base);  \
    }  \
  } while (0)

  LOPARTICLE_TYPE_EACH_(each_);
  return false;

# undef each_
}

static void loparticle_base_draw_(
    loentity_t* entity, const locommon_position_t* basepos) {
  assert(entity  != NULL);
  assert(basepos != NULL);

  loparticle_base_t* base = (typeof(base)) entity;

  vec2_t p;
  locommon_position_sub(&p, &base->super.pos, basepos);
  vec2_addeq(&base->cache.instance.pos, &p);

  loshader_particle_drawer_add_instance(base->drawer, &base->cache.instance);
}

static void loparticle_base_pack_(
    const loentity_t* entity, msgpack_packer* packer) {
  assert(entity != NULL);
  assert(packer != NULL);

  const loparticle_base_t* base = (typeof(base)) entity;

  msgpack_pack_map(packer, CRIAL_PROPERTY_COUNT_+1);

  CRIAL_SERIALIZER_;

  mpkutil_pack_str(packer, "data");
# define each_(NAME, name) do {  \
    if (base->type == LOPARTICLE_TYPE_##NAME) {  \
      loparticle_##name##_pack_data(base, packer);  \
      return;  \
    }  \
  } while (0)

  LOPARTICLE_TYPE_EACH_(each_);
  assert(false);

# undef each_
}

void loparticle_base_initialize(
    loparticle_base_t*          base,
    loshader_particle_drawer_t* drawer,
    const locommon_ticker_t*    ticker,
    loentity_store_t*           entities) {
  assert(base     != NULL);
  assert(drawer   != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);

  *base = (typeof(*base)) {
    .super = {
      .vtable = {
        .delete = loparticle_base_delete_,
        .die    = loparticle_base_die_,
        .update = loparticle_base_update_,
        .draw   = loparticle_base_draw_,
        .pack   = loparticle_base_pack_,
      },
      .subclass = LOENTITY_SUBCLASS_PARTICLE,
    },
    .drawer   = drawer,
    .ticker   = ticker,
    .entities = entities,
  };
}

void loparticle_base_reinitialize(loparticle_base_t* base, loentity_id_t id) {
  assert(base != NULL);

  base->super.id = id;
}

void loparticle_base_deinitialize(loparticle_base_t* base) {
  assert(base != NULL);

  loparticle_base_delete_(&base->super);
}

bool loparticle_base_unpack(
    loparticle_base_t* base, const msgpack_object* obj) {
  assert(base != NULL);

  loparticle_base_reinitialize(base, 0);

  const msgpack_object_map* root = mpkutil_get_map(obj);

  CRIAL_DESERIALIZER_;

  const msgpack_object* data = mpkutil_get_map_item_by_str(root, "data");
# define each_(NAME, name) do {  \
    if (base->type == LOPARTICLE_TYPE_##NAME) {  \
      if (!loparticle_##name##_unpack_data(base, data)) return false;  \
    }  \
  } while (0)

  LOPARTICLE_TYPE_EACH_(each_);
  return true;

# undef each_
}
