#include "./base.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/msgpack.h"
#include "core/locommon/position.h"
#include "core/loentity/entity.h"
#include "core/loentity/ground.h"
#include "core/loshader/ground.h"

#include "./island.h"
#include "./type.h"

/* generated serializer */
#include "core/loground/crial/base.h"

static bool
(*const update_function_vtable_[LOGROUND_TYPE_COUNT])(loground_base_t* base) = {
  [LOGROUND_TYPE_ISLAND] = loground_island_update,
};

static void loground_base_delete_(loentity_t* entity) {
  assert(entity != NULL);

  loground_base_t* base = (typeof(base)) entity;
  if (!base->used) return;

  base->used = false;
}

static void loground_base_die_(loentity_t* entity) {
  assert(entity != NULL);

}

static bool loground_base_update_(loentity_t* entity) {
  assert(entity != NULL);

  loground_base_t* base = (typeof(base)) entity;
  base->cache = (typeof(base->cache)) {0};

  assert(update_function_vtable_[base->param.type] != NULL);
  return update_function_vtable_[base->param.type](base);
}

static void loground_base_draw_(
    loentity_t* entity, const locommon_position_t* basepos) {
  assert(entity != NULL);
  assert(locommon_position_valid(basepos));

  loground_base_t* base = (typeof(base)) entity;

  vec2_t p;
  locommon_position_sub(&p, &base->super.super.pos, basepos);
  vec2_addeq(&base->cache.instance.pos, &p);

  loshader_ground_drawer_add_instance(base->drawer, &base->cache.instance);
}

static void loground_base_pack_(
    const loentity_t* entity, msgpack_packer* packer) {
  assert(entity != NULL);
  assert(packer != NULL);

  const loground_base_t* base = (typeof(base)) entity;

  msgpack_pack_map(packer, CRIAL_PROPERTY_COUNT_);
  CRIAL_SERIALIZER_;
}

void loground_base_initialize(
    loground_base_t* base, loshader_ground_drawer_t* drawer) {
  assert(base != NULL);
  assert(drawer != NULL);

  *base = (typeof(*base)) {
    .drawer = drawer,
  };
}

void loground_base_reinitialize(loground_base_t* base, loentity_id_t id) {
  assert(base != NULL);
  assert(!base->used);

  base->super = (typeof(base->super)) {
    .super = {
      .vtable = {
        .delete = loground_base_delete_,
        .die    = loground_base_die_,
        .update = loground_base_update_,
        .draw   = loground_base_draw_,
        .pack   = loground_base_pack_,
      },
      .id       = id,
      .subclass = LOENTITY_SUBCLASS_GROUND,
    },
  };
  base->param = (typeof(base->param)) {0};
}

void loground_base_deinitialize(loground_base_t* base) {
  assert(base != NULL);
  assert(!base->used);

}

bool loground_base_unpack(loground_base_t* base, const msgpack_object *obj) {
  assert(base != NULL);
  assert(obj  != NULL);

  loground_base_reinitialize(base, 0);  /* id will be overwritten */

  const msgpack_object_map* root = mpkutil_get_map(obj);
  if (root == NULL) goto FAIL;
  CRIAL_DESERIALIZER_;
  return true;

FAIL:
  loground_base_delete_(&base->super.super);
  return false;
}
