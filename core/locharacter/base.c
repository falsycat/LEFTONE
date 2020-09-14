#include "./base.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "core/lobullet/pool.h"
#include "core/locommon/easing.h"
#include "core/locommon/msgpack.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/recipient.h"
#include "core/loentity/character.h"
#include "core/loentity/store.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/character.h"

#include "./big_warder.h"
#include "./cavia.h"
#include "./encephalon.h"
#include "./greedy_scientist.h"
#include "./scientist.h"
#include "./theists_child.h"
#include "./misc.h"
#include "./warder.h"

#define LOCHARACTER_BASE_PARAM_TO_PACK_EACH_(  \
    PROC, PROC_type, PROC_state, PROC_str) do {  \
  PROC_str  ("subclass",            "character");  \
  PROC_type ("type",                type);  \
  PROC      ("id",                  super.super.id);  \
  PROC      ("ground",              ground);  \
  PROC      ("pos",                 pos);  \
  PROC      ("direction",           direction);  \
  PROC      ("knockback",           knockback);  \
  PROC      ("gravity",             gravity);  \
  PROC      ("madness",             recipient.madness);  \
  PROC      ("effects",             recipient.effects);  \
  PROC_state("state",               state);  \
  PROC      ("since",               since);  \
  PROC      ("last-update-time",    last_update_time);  \
  PROC      ("last-knockback-time", last_knockback_time);  \
  PROC      ("last-hit-time",       last_hit_time);  \
} while (0)
#define LOCHARACTER_BASE_PARAM_TO_PACK_COUNT 15

static void locharacter_base_convert_to_world_pos_(
    const loentity_ground_t* g, locommon_position_t* wpos, const vec2_t* pos) {
  assert(g    != NULL);
  assert(wpos != NULL);
  assert(vec2_valid(pos));

  vec2_t p = *pos;
  p.x *= g->size.x;
  p.y += g->size.y;

  *wpos = g->super.pos;
  vec2_addeq(&wpos->fract, &p);
  locommon_position_reduce(wpos);
}

static void locharacter_base_convert_from_world_pos_(
    const loentity_ground_t* g, vec2_t* pos, const locommon_position_t* wpos) {
  assert(g   != NULL);
  assert(pos != NULL);
  assert(locommon_position_valid(wpos));

  locommon_position_sub(pos, wpos, &g->super.pos);
  pos->x /= g->size.x;
  pos->y -= g->size.y;
}

static loentity_ground_t* locharacter_base_get_ground_(locharacter_base_t* base) {
  assert(base != NULL);

  loentity_store_iterator_t itr;
  if (loentity_store_find_item_by_id(base->entities, &itr, base->ground)) {
    return itr.ground;
  }
  return NULL;
}

static void locharacter_base_handle_knockback_(locharacter_base_t* base) {
  assert(base != NULL);

  vec2_t v = base->knockback;
  v.x /= base->cache.ground->size.x;
  vec2_muleq(&v, base->ticker->delta_f);
  vec2_addeq(&base->pos, &v);

  locommon_easing_linear_float(&base->knockback.x, 0, base->ticker->delta_f/2);
  locommon_easing_linear_float(&base->knockback.y, 0, base->ticker->delta_f/2);
}

static void locharacter_base_calculate_world_position_(
    locharacter_base_t* base) {
  assert(base != NULL);

  base->pos.x = MATH_CLAMP(base->pos.x, -1, 1);
  base->pos.y = MATH_CLAMP(base->pos.y,  0, 1);

  if (base->pos.y < base->cache.height) {
    if (base->cache.gravity) base->gravity = 0;
    base->pos.y = base->cache.height;
  }
  locharacter_base_convert_to_world_pos_(
      base->cache.ground, &base->super.super.pos, &base->pos);
}

static void locharacter_base_calculate_velocity_(
    locharacter_base_t* base, vec2_t* v, const locommon_position_t* oldpos) {
  assert(base   != NULL);
  assert(v      != NULL);
  assert(locommon_position_valid(oldpos));

  locommon_position_sub(v, &base->super.super.pos, oldpos);
  vec2_diveq(v, base->ticker->delta_f);
}

static void locharacter_base_execute_bullet_hittest_(
    locharacter_base_t* base, const vec2_t* velocity) {
  assert(base != NULL);
  assert(vec2_valid(velocity));

  if (base->last_hit_time + 200 > base->ticker->time) return;

  if (loentity_store_affect_bullets_shot_by_one(
        base->entities,
        &base->super,
        base->player->entity.super.super.id,
        velocity,
        base->ticker->delta_f)) {
    base->last_hit_time = base->ticker->time;
  }
}

static void locharacter_base_delete_(loentity_t* entity) {
  assert(entity != NULL);

  locharacter_base_t* base = (typeof(base)) entity;
  if (!base->used) return;

  base->used = false;

# define each_(NAME, name) do {  \
    if (base->type == LOCHARACTER_TYPE_##NAME) {  \
      locharacter_##name##_tear_down(base);  \
      return;  \
    }  \
  } while (0)

  LOCHARACTER_TYPE_EACH_(each_);
  assert(false);

# undef each_
}

static void locharacter_base_die_(loentity_t* entity) {
  assert(entity != NULL);

}

static bool locharacter_base_update_(loentity_t* entity) {
  assert(entity != NULL);

  static const float gravity_acceleration = 2.f;

  locharacter_base_t* base = (typeof(base)) entity;

  base->cache = (typeof(base->cache)) {
    .time = base->ticker->time,
  };

  base->cache.ground = locharacter_base_get_ground_(base);
  if (base->cache.ground == NULL) return false;

  locharacter_base_convert_from_world_pos_(
      base->cache.ground,
      &base->cache.player_pos,
      &base->player->entity.super.super.pos);

  locharacter_base_handle_knockback_(base);

  locommon_position_t oldpos = base->super.super.pos;

  base->pos.y += base->gravity * base->ticker->delta_f;

# define each_(NAME, name) do {  \
    if (base->type == LOCHARACTER_TYPE_##NAME) {  \
      if (!locharacter_##name##_update(base)) return false;  \
    }  \
  } while (0)

  LOCHARACTER_TYPE_EACH_(each_);

# undef each_

  locharacter_base_calculate_world_position_(base);

  if (base->cache.gravity) {
    base->gravity -= base->ticker->delta_f * gravity_acceleration;
  } else {
    base->gravity = 0;
  }
  if (base->cache.bullet_hittest) {
    vec2_t velocity;
    locharacter_base_calculate_velocity_(base, &velocity, &oldpos);
    locharacter_base_execute_bullet_hittest_(base, &velocity);
  }

  base->cache.ground     = NULL;
  base->last_update_time = base->cache.time;
  return true;
}

static void locharacter_base_draw_(
    loentity_t* entity, const locommon_position_t* basepos) {
  assert(entity != NULL);
  assert(locommon_position_valid(basepos));

  locharacter_base_t* base = (typeof(base)) entity;

  vec2_t v;
  locommon_position_sub(&v, &base->super.super.pos, basepos);
  vec2_addeq(&base->cache.instance.pos, &v);

  loshader_character_drawer_add_instance(base->drawer, &base->cache.instance);
}

static void locharacter_base_apply_effect_(
    loentity_character_t* entity, const loeffect_t* effect) {
  assert(entity   != NULL);
  assert(effect != NULL);

  locharacter_base_t* base = (typeof(base)) entity;
  loeffect_recipient_apply_effect(&base->recipient, effect);
}

static void locharacter_base_knockback_(
    loentity_character_t* chara, const vec2_t* knockback) {
  assert(chara != NULL);
  assert(vec2_valid(knockback));

  locharacter_base_t* base = (typeof(base)) chara;

  static const float r = .05f;
  if (vec2_pow_length(knockback) > r*r) {
    base->last_knockback_time = base->ticker->time;
  }
  vec2_addeq(&base->knockback, knockback);
}

static void locharacter_base_pack_(
    const loentity_t* chara, msgpack_packer* packer) {
  assert(chara  != NULL);
  assert(packer != NULL);

  const locharacter_base_t* base = (typeof(base)) chara;

  msgpack_pack_map(packer, LOCHARACTER_BASE_PARAM_TO_PACK_COUNT+1);

# define pack_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &base->var);  \
  } while (0)
# define pack_type_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    mpkutil_pack_str(packer, locharacter_type_stringify(base->var));  \
  } while (0)
# define pack_state_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    mpkutil_pack_str(packer, locharacter_state_stringify(base->var));  \
  } while (0)
# define pack_str_(name, str) do {  \
    mpkutil_pack_str(packer, name);  \
    mpkutil_pack_str(packer, str);  \
  } while (0)

  LOCHARACTER_BASE_PARAM_TO_PACK_EACH_(pack_, pack_type_, pack_state_, pack_str_);

# undef pack_str_
# undef pack_state_
# undef pack_type_
# undef pack_

# define each_(NAME, name) do {  \
    if (base->type == LOCHARACTER_TYPE_##NAME) {  \
      locharacter_##name##_pack_data(base, packer);  \
      return;  \
    }  \
  } while (0)

  mpkutil_pack_str(packer, "data");
  LOCHARACTER_TYPE_EACH_(each_);
  assert(false);

# undef each_
}

void locharacter_base_initialize(
    locharacter_base_t*          base,
    loresource_set_t*            res,
    loshader_character_drawer_t* drawer,
    const locommon_ticker_t*     ticker,
    lobullet_pool_t*             bullets,
    loentity_store_t*            entities,
    loplayer_t*                  player) {
  assert(base     != NULL);
  assert(res      != NULL);
  assert(drawer   != NULL);
  assert(ticker   != NULL);
  assert(bullets  != NULL);
  assert(entities != NULL);
  assert(player   != NULL);

  *base = (typeof(*base)) {
    .super = {
      .super = {
        .vtable = {
          .delete = locharacter_base_delete_,
          .die    = locharacter_base_die_,
          .update = locharacter_base_update_,
          .draw   = locharacter_base_draw_,
          .pack   = locharacter_base_pack_,
        },
        .subclass = LOENTITY_SUBCLASS_CHARACTER,
      },
      .vtable = {
        .apply_effect = locharacter_base_apply_effect_,
        .knockback    = locharacter_base_knockback_,
      },
    },
    .res      = res,
    .drawer   = drawer,
    .ticker   = ticker,
    .bullets  = bullets,
    .entities = entities,
    .player   = player,
  };
  loeffect_recipient_initialize(&base->recipient, ticker);
}

void locharacter_base_reinitialize(locharacter_base_t* base, loentity_id_t id) {
  assert(base != NULL);

# define reset_(name, var) do {  \
    base->var = (typeof(base->var)) {0};  \
  } while (0)
# define reset_str_(name, str)

  LOCHARACTER_BASE_PARAM_TO_PACK_EACH_(
      reset_, reset_, reset_, reset_str_);

# undef reset_str_
# undef reset_

  loeffect_recipient_reset(&base->recipient);
  base->super.super.id = id;
}

void locharacter_base_deinitialize(locharacter_base_t* base) {
  assert(base != NULL);

  if (base->used) locharacter_base_delete_(&base->super.super);
  loeffect_recipient_deinitialize(&base->recipient);
}

bool locharacter_base_unpack(
    locharacter_base_t* base, const msgpack_object* obj) {
  assert(base != NULL);
  assert(obj  != NULL);

  locharacter_base_reinitialize(base, 0);
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
        !locharacter_type_unstringify(&base->var, v, vlen)) {  \
      return NULL;  \
    }  \
  } while (0)
# define unpack_state_(name, var) do {  \
    if (!mpkutil_get_str(item_(name), &v, &vlen) ||  \
        !locharacter_state_unstringify(&base->var, v, vlen)) {  \
      return NULL;  \
    }  \
  } while (0)
# define unpack_str_(name, str) do {  \
    if (!mpkutil_get_str(item_(name), &v, &vlen) ||  \
        !(strncmp(v, str, vlen) == 0 && str[vlen] == 0)) {  \
      return NULL;  \
    }  \
  } while (0)

  LOCHARACTER_BASE_PARAM_TO_PACK_EACH_(
      unpack_, unpack_type_, unpack_state_, unpack_str_);

# undef unpack_str_
# undef unpack_state_
# undef unpack_type_
# undef unpack_

  const msgpack_object* data = item_("data");

# define each_(NAME, name) do {  \
    if (base->type == LOCHARACTER_TYPE_##NAME) {  \
      return locharacter_##name##_unpack_data(base, data);  \
    }  \
  } while (0)

  LOCHARACTER_TYPE_EACH_(each_);
  return false;

# undef each_

# undef item_
}
