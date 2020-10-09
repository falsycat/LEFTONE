#include "./base.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/msgpack.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loentity/bullet.h"
#include "core/loentity/character.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"
#include "core/loresource/set.h"
#include "core/loshader/bullet.h"
#include "core/loshader/set.h"

#include "./linear.h"
#include "./type.h"

/* generated serializer */
#include "core/lobullet/crial/base.h"

static bool
(*const update_function_vtable_[LOBULLET_TYPE_COUNT])(lobullet_base_t* base) = {
  [LOBULLET_TYPE_LINEAR_CIRCLE]   = lobullet_linear_circle_update,
  [LOBULLET_TYPE_LINEAR_TRIANGLE] = lobullet_linear_triangle_update,
  [LOBULLET_TYPE_LINEAR_SQUARE]   = lobullet_linear_square_update,
};

static void lobullet_base_delete_(loentity_t* entity) {
  assert(entity != NULL);

  lobullet_base_t* base = (typeof(base)) entity;
  if (!base->used) return;

  base->used = false;
}

static void lobullet_base_die_(loentity_t* entity) {
  assert(entity != NULL);

}

static bool lobullet_base_update_(loentity_t* entity) {
  assert(entity != NULL);

  lobullet_base_t* base = (typeof(base)) entity;

  base->cache = (typeof(base->cache)) {0};
  base->super.owner    = base->param.owner;
  base->super.velocity = vec2(0, 0);

  const locommon_position_t oldpos = base->super.super.pos;

  assert(update_function_vtable_[base->param.type] != NULL);
  if (!update_function_vtable_[base->param.type](base)) {
    return false;
  }
  if (base->cache.velocity_calc) {
    locommon_position_sub(
        &base->super.velocity, &base->super.super.pos, &oldpos);
  }
  return true;
}

static void lobullet_base_draw_(
    loentity_t* entity, const locommon_position_t* basepos) {
  assert(entity != NULL);
  assert(locommon_position_valid(basepos));

  lobullet_base_t* base = (typeof(base)) entity;

  vec2_t p;
  locommon_position_sub(&p, &base->super.super.pos, basepos);
  vec2_addeq(&base->cache.instance.pos, &p);

  loshader_bullet_drawer_add_instance(
      &base->shaders->drawer.bullet, &base->cache.instance);
}

static void lobullet_base_pack_(
    const loentity_t* entity, msgpack_packer* packer) {
  assert(entity != NULL);
  assert(packer != NULL);

  const lobullet_base_t* base = (typeof(base)) entity;

  msgpack_pack_map(packer, CRIAL_PROPERTY_COUNT_);
  CRIAL_SERIALIZER_;
}

static bool lobullet_base_affect_(
    loentity_bullet_t* bullet, loentity_character_t* chara) {
  assert(bullet != NULL);

  lobullet_base_t* base = (typeof(base)) bullet;

  vec2_t v = vec2(0, 0);
  locommon_position_sub(&v, &chara->super.pos, &base->super.super.pos);
  vec2_muleq(&v, base->cache.knockback);
  loentity_character_knockback(chara, &v);

  if (base->cache.toxic) {
    loentity_character_apply_effect(chara, &base->param.effect);
  }
  return base->cache.toxic;
}

void lobullet_base_initialize(
    lobullet_base_t*         base,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities) {
  assert(base     != NULL);
  assert(res      != NULL);
  assert(shaders  != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);

  *base = (typeof(*base)) {
    .res      = res,
    .shaders  = shaders,
    .ticker   = ticker,
    .entities = entities,
  };
}

void lobullet_base_reinitialize(lobullet_base_t* base, loentity_id_t id) {
  assert(base != NULL);
  assert(!base->used);

  base->super = (typeof(base->super)) {
    .super = {
      .vtable = {
        .delete = lobullet_base_delete_,
        .die    = lobullet_base_die_,
        .update = lobullet_base_update_,
        .draw   = lobullet_base_draw_,
        .pack   = lobullet_base_pack_,
      },
      .id       = id,
      .subclass = LOENTITY_SUBCLASS_BULLET,
    },
    .vtable = {
      .affect = lobullet_base_affect_,
    },
  };

  base->param = (typeof(base->param)) {0};
}

void lobullet_base_deinitialize(lobullet_base_t* base) {
  assert(base != NULL);
  assert(!base->used);

  lobullet_base_delete_(&base->super.super);
}

bool lobullet_base_unpack(lobullet_base_t* base, const msgpack_object* obj) {
  assert(base != NULL);

  lobullet_base_reinitialize(base, 0);

  const msgpack_object_map* root = mpkutil_get_map(obj);
  if (root == NULL) goto FAIL;

  CRIAL_DESERIALIZER_;
  return true;

FAIL:
  lobullet_base_delete_(&base->super.super);
  return false;
}
