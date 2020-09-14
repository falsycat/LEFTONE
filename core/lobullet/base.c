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

#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loentity/bullet.h"
#include "core/loentity/character.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"
#include "core/loresource/set.h"
#include "core/loshader/bullet.h"

#include "./bomb.h"
#include "./linear.h"
#include "./misc.h"

static void lobullet_base_delete_(loentity_t* entity) {
  assert(entity != NULL);

  lobullet_base_t* base = (typeof(base)) entity;
  if (!base->used) return;

  base->used = false;

# define each_(NAME, name) do {  \
    if (base->type == LOBULLET_TYPE_##NAME) {  \
      lobullet_##name##_tear_down(base);  \
      return;  \
    }  \
  } while (0)

  LOBULLET_TYPE_EACH_(each_);
  assert(false);

# undef each_
}

static void lobullet_base_die_(loentity_t* entity) {
  assert(entity != NULL);

}

static bool lobullet_base_update_(loentity_t* entity) {
  assert(entity != NULL);

  lobullet_base_t* base = (typeof(base)) entity;
  base->cache = (typeof(base->cache)) {0};

# define each_(NAME, name) do {  \
    if (base->type == LOBULLET_TYPE_##NAME) {  \
      return lobullet_##name##_update(base);  \
    }  \
  } while (0)

  LOBULLET_TYPE_EACH_(each_);
  return false;

# undef each_
}

static void lobullet_base_draw_(
    loentity_t* entity, const locommon_position_t* basepos) {
  assert(entity != NULL);
  assert(locommon_position_valid(basepos));

  lobullet_base_t* base = (typeof(base)) entity;

  vec2_t p;
  locommon_position_sub(&p, &base->super.super.pos, basepos);
  vec2_addeq(&base->cache.instance.pos, &p);

  loshader_bullet_drawer_add_instance(base->drawer, &base->cache.instance);
}

static void lobullet_base_pack_(
    const loentity_t* entity, msgpack_packer* packer) {
  assert(entity != NULL);
  assert(packer != NULL);

  const lobullet_base_t* base = (typeof(base)) entity;

  msgpack_pack_map(packer, 4);

  mpkutil_pack_str(packer, "subclass");
  mpkutil_pack_str(packer, "bullet");

  mpkutil_pack_str(packer, "type");
  mpkutil_pack_str(packer, lobullet_type_stringify(base->type));

  mpkutil_pack_str(packer, "id");
  msgpack_pack_uint64(packer, base->super.super.id);

  mpkutil_pack_str(packer, "data");
# define each_(NAME, name) do {  \
    if (base->type == LOBULLET_TYPE_##NAME) {  \
      lobullet_##name##_pack_data(base, packer);  \
      return;  \
    }  \
  } while (0)

  LOBULLET_TYPE_EACH_(each_);
  assert(false);

# undef each_
}

static bool lobullet_base_affect_(
    loentity_bullet_t* bullet, loentity_character_t* chara) {
  assert(bullet != NULL);

  lobullet_base_t* base = (typeof(base)) bullet;

  vec2_t v = vec2(0, 0);
  switch (base->cache.knockback.algorithm) {
  case LOBULLET_BASE_KNOCKBACK_ALGORITHM_VELOCITY:
    v = base->super.velocity;
    break;
  case LOBULLET_BASE_KNOCKBACK_ALGORITHM_POSITION:
    locommon_position_sub(&v, &chara->super.pos, &base->super.super.pos);
    break;
  }
  const float plen = vec2_pow_length(&v);
  if (plen != 0) {
    vec2_diveq(&v, sqrtf(plen));
    vec2_muleq(&v, base->cache.knockback.acceleration);
    loentity_character_knockback(chara, &v);
  }

  if (base->cache.toxic) {
    loentity_character_apply_effect(chara, &base->cache.effect);
  }
  return base->cache.toxic;
}

void lobullet_base_initialize(
    lobullet_base_t*          base,
    loresource_set_t*         res,
    loshader_bullet_drawer_t* drawer,
    const locommon_ticker_t*  ticker,
    loentity_store_t*         entities) {
  assert(base     != NULL);
  assert(res      != NULL);
  assert(drawer   != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);

  *base = (typeof(*base)) {
    .super = {
      .super = {
        .vtable = {
          .delete = lobullet_base_delete_,
          .die    = lobullet_base_die_,
          .update = lobullet_base_update_,
          .draw   = lobullet_base_draw_,
          .pack   = lobullet_base_pack_,
        },
        .subclass = LOENTITY_SUBCLASS_BULLET,
      },
      .vtable = {
        .affect = lobullet_base_affect_,
      },
    },
    .res      = res,
    .drawer   = drawer,
    .ticker   = ticker,
    .entities = entities,
  };
}

void lobullet_base_reinitialize(lobullet_base_t* base, loentity_id_t id) {
  assert(base != NULL);

  base->super.super.id = id;
}

void lobullet_base_deinitialize(lobullet_base_t* base) {
  assert(base != NULL);

  lobullet_base_delete_(&base->super.super);
}

bool lobullet_base_unpack(lobullet_base_t* base, const msgpack_object* obj) {
  assert(base != NULL);

  lobullet_base_reinitialize(base, 0);

  const char* v;
  size_t      vlen;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define streq_(v1, len, v2)   \
    (strncmp(v1, v2, len) == 0 && v2[len] == 0)
  if (!mpkutil_get_str(item_("subclass"), &v, &vlen) ||
      !streq_(v, vlen, "bullet")) {
    return false;
  }
# undef streq_

  if (!mpkutil_get_str(item_("type"), &v, &vlen) ||
      !lobullet_type_unstringify(&base->type, v, vlen)) {
    return false;
  }

  if (!mpkutil_get_uint64(item_("id"), &base->super.super.id)) {
    return false;
  }

  const msgpack_object* data = item_("data");
# define each_(NAME, name) do {  \
    if (base->type == LOBULLET_TYPE_##NAME) {  \
      if (!lobullet_##name##_unpack_data(base, data)) return false;  \
    }  \
  } while (0)

  LOBULLET_TYPE_EACH_(each_);

# undef each_

# undef item_
  return true;
}
