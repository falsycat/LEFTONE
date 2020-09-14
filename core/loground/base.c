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
#include "./misc.h"

#define LOGROUND_BASE_PARAM_TO_PACK_EACH_(PROC, PROC_str, PROC_type) do {  \
  PROC_str  ("subclass", "ground");  \
  PROC_type ("type",     type);  \
  PROC      ("id",       super.super.id);  \
  PROC      ("pos",      super.super.pos);  \
  PROC      ("size",     super.size);  \
} while (0)
#define LOGROUND_BASE_PARAM_TO_PACK_COUNT 5

static void loground_base_delete_(loentity_t* entity) {
  assert(entity != NULL);

  loground_base_t* base = (typeof(base)) entity;
  if (!base->used) return;

  base->used = false;

# define each_(NAME, name) do {  \
    if (base->type == LOGROUND_TYPE_##NAME) {  \
      loground_##name##_tear_down(base);  \
      return;  \
    }  \
  } while (0)

  LOGROUND_TYPE_EACH_(each_);
  assert(false);

# undef each_
}

static void loground_base_die_(loentity_t* entity) {
  assert(entity != NULL);

}

static bool loground_base_update_(loentity_t* entity) {
  assert(entity != NULL);

  loground_base_t* base = (typeof(base)) entity;
  base->cache = (typeof(base->cache)) {0};

# define each_(NAME, name) do {  \
    if (base->type == LOGROUND_TYPE_##NAME) {  \
      return loground_##name##_update(base);  \
    }  \
  } while (0)

  LOGROUND_TYPE_EACH_(each_);
  return false;

# undef each_
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

  msgpack_pack_map(packer, LOGROUND_BASE_PARAM_TO_PACK_COUNT+1);

# define pack_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &base->var);  \
  } while (0)
# define pack_str_(name, str) do {  \
    mpkutil_pack_str(packer, name);  \
    mpkutil_pack_str(packer, str);  \
  } while (0)
# define pack_type_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    mpkutil_pack_str(packer, loground_type_stringify(base->var));  \
  } while (0)


  LOGROUND_BASE_PARAM_TO_PACK_EACH_(pack_, pack_str_, pack_type_);

# undef pack_type_
# undef pack_str_
# undef pack_

# define each_(NAME, name) do {  \
    if (base->type == LOGROUND_TYPE_##NAME) {  \
      loground_##name##_pack_data(base, packer);  \
      return;  \
    }  \
  } while (0)

  mpkutil_pack_str(packer, "data");
  LOGROUND_TYPE_EACH_(each_);
  assert(false);

# undef each_
}

void loground_base_initialize(
    loground_base_t* base, loshader_ground_drawer_t* drawer) {
  assert(base != NULL);
  assert(drawer != NULL);

  *base = (typeof(*base)) {
    .super = {
      .super = {
        .vtable = {
          .delete = loground_base_delete_,
          .die    = loground_base_die_,
          .update = loground_base_update_,
          .draw   = loground_base_draw_,
          .pack   = loground_base_pack_,
        },
        .subclass = LOENTITY_SUBCLASS_GROUND,
      },
    },
    .drawer = drawer,
  };
}

void loground_base_reinitialize(loground_base_t* base, loentity_id_t id) {
  assert(base != NULL);

# define reset_(name, var) do {  \
    base->var = (typeof(base->var)) {0};  \
  } while (0)
# define reset_str_(name, str)

  LOGROUND_BASE_PARAM_TO_PACK_EACH_(reset_, reset_str_, reset_);

# undef reset_str_
# undef reset_

  base->super.super.id = id;
}

void loground_base_deinitialize(loground_base_t* base) {
  assert(base != NULL);

  loground_base_delete_(&base->super.super);
}

bool loground_base_unpack(loground_base_t* base, const msgpack_object *obj) {
  assert(base != NULL);
  assert(obj  != NULL);

  loground_base_reinitialize(base, 0);
  /* id will be overwritten below */

  const char* v;
  size_t      vlen;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define unpack_(name, var) do {  \
    if (!LOCOMMON_MSGPACK_UNPACK_ANY(item_(name), &base->var)) {  \
      return NULL;  \
    }  \
  } while (0)
# define unpack_type_(name, var) do {  \
    if (!mpkutil_get_str(item_(name), &v, &vlen) ||  \
        !loground_type_unstringify(&base->var, v, vlen)) {  \
      return NULL;  \
    }  \
  } while (0)
# define unpack_str_(name, str) do {  \
    if (!mpkutil_get_str(item_(name), &v, &vlen) ||  \
        !(strncmp(v, str, vlen) == 0 && str[vlen] == 0)) {  \
      return NULL;  \
    }  \
  } while (0)

  LOGROUND_BASE_PARAM_TO_PACK_EACH_(unpack_, unpack_str_, unpack_type_);

# undef unpack_str_
# undef unpack_type_
# undef unpack_

  const msgpack_object* data = item_("data");
# define each_(NAME, name) do {  \
    if (base->type == LOGROUND_TYPE_##NAME) {  \
      return loground_##name##_unpack_data(base, data);  \
    }  \
  } while (0)

  LOGROUND_TYPE_EACH_(each_);
  return false;

# undef each_

# undef item_
}
