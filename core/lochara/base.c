#include "./base.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"
#include "util/statman/statman.h"

#include "core/lobullet/pool.h"
#include "core/locommon/easing.h"
#include "core/locommon/msgpack.h"
#include "core/locommon/physics.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/recipient.h"
#include "core/loentity/character.h"
#include "core/loentity/ground.h"
#include "core/loentity/store.h"
#include "core/loplayer/player.h"
#include "core/loresource/sound.h"
#include "core/loshader/character.h"

#include "./big_warder.h"
#include "./cavia.h"
#include "./encephalon.h"
#include "./player.h"
#include "./state.h"
#include "./theists_child.h"
#include "./type.h"
#include "./warder.h"

/* generated serializer */
#include "core/lochara/crial/base.h"

#define GRAVITY_ACCELERATION_            2.2f
#define KNOCKBACK_RECOVERY_ACCELERATION_ 4.4f

#define BULLET_INVINCIBLE_DURATION_ 500

static bool
(*const update_function_vtable_[LOCHARA_TYPE_COUNT])(lochara_base_t* base) = {
  [LOCHARA_TYPE_PLAYER]     = lochara_player_update,

  [LOCHARA_TYPE_ENCEPHALON] = lochara_encephalon_update,

  [LOCHARA_TYPE_CAVIA]  = lochara_cavia_update,
  [LOCHARA_TYPE_WARDER] = lochara_warder_update,

  [LOCHARA_TYPE_BIG_WARDER]    = lochara_big_warder_update,
  [LOCHARA_TYPE_THEISTS_CHILD] = lochara_theists_child_update,
};

static loentity_ground_t* lochara_base_find_ground_(
    lochara_base_t* base, loentity_id_t id, vec2_t* pos) {
  assert(base != NULL);
  assert(pos  != NULL);

  loentity_store_iterator_t itr;
  if (!loentity_store_find_item_by_id(base->entities, &itr, id) ||
      itr.ground == NULL) {
    return NULL;
  }
  locommon_position_sub(pos, &base->super.super.pos, &itr.ground->super.pos);
  pos->x /= itr.ground->size.x;
  pos->y -= itr.ground->size.y;
  return itr.ground;
}

static void lochara_base_delete_(loentity_t* entity) {
  assert(entity != NULL);

  lochara_base_t* base = (typeof(base)) entity;

  loeffect_recipient_deinitialize(&base->param.recipient);

  base->used = false;
}

static void lochara_base_die_(loentity_t* entity) {
  assert(entity != NULL);

}

static bool lochara_base_update_(loentity_t* entity) {
  assert(entity != NULL);

  lochara_base_t* base = (typeof(base)) entity;

  base->cache = (typeof(base->cache)) {0};
  base->super.velocity = vec2(0, 0);

  base->cache.ground = lochara_base_find_ground_(
      base, base->param.ground, &base->cache.ground_pos);

  assert(update_function_vtable_[base->param.type] != NULL);
  return update_function_vtable_[base->param.type](base);
}

static void lochara_base_draw_(
    loentity_t* entity, const locommon_position_t* basepos) {
  assert(entity != NULL);
  assert(locommon_position_valid(basepos));

  lochara_base_t* base = (typeof(base)) entity;

  vec2_t pos;
  locommon_position_sub(&pos, &base->super.super.pos, basepos);
  vec2_addeq(&base->cache.instance.pos, &pos);

  loshader_character_drawer_add_instance(
      &base->shaders->drawer.character, &base->cache.instance);
}

static void lochara_base_pack_(
    const loentity_t* entity, msgpack_packer* packer) {
  assert(entity != NULL);
  assert(packer != NULL);

  lochara_base_t* base = (typeof(base)) entity;

  msgpack_pack_map(packer, CRIAL_PROPERTY_COUNT_);
  CRIAL_SERIALIZER_;
}

static void lochara_base_apply_effect_(
    loentity_character_t* chara, const loeffect_t* effect) {
  assert(chara  != NULL);
  assert(effect != NULL);

  lochara_base_t* base = (typeof(base)) chara;

  const bool player = base->param.type == LOCHARA_TYPE_PLAYER;

  loeffect_recipient_apply_effect(&base->param.recipient, effect);

  switch (effect->id) {
  case LOEFFECT_ID_DAMAGE:
    if (player) {
      loresource_sound_set_play(&base->res->sound, LORESOURCE_SOUND_ID_DAMAGE);
    }
    break;
  default:
    break;
  }
}

static void lochara_base_knockback_(
    loentity_character_t* chara, const vec2_t* v) {
  assert(chara != NULL);
  assert(vec2_valid(v));

  lochara_base_t* base = (typeof(base)) chara;

  base->param.gravity   += v->y;
  base->param.knockback += v->x;

  if (vec2_pow_length(v) > 0) {
    base->param.last_knockback = base->ticker->time;
  }
}

void lochara_base_initialize(
    lochara_base_t*          base,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities,
    loplayer_t*              player,
    lobullet_pool_t*         bullet) {
  assert(base     != NULL);
  assert(res      != NULL);
  assert(shaders  != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);
  assert(player   != NULL);
  assert(bullet   != NULL);

  *base = (typeof(*base)) {
    .res      = res,
    .shaders  = shaders,
    .ticker   = ticker,
    .entities = entities,
    .player   = player,
    .bullet   = bullet,
  };
}

void lochara_base_reinitialize(lochara_base_t* base, loentity_id_t id) {
  assert(base != NULL);
  assert(!base->used);

  base->super = (typeof(base->super)) {
    .super = {
      .vtable = {
        .delete = lochara_base_delete_,
        .die    = lochara_base_die_,
        .update = lochara_base_update_,
        .draw   = lochara_base_draw_,
        .pack   = lochara_base_pack_,
      },
      .id       = id,
      .subclass = LOENTITY_SUBCLASS_CHARACTER,
    },
    .vtable = {
      .apply_effect = lochara_base_apply_effect_,
      .knockback    = lochara_base_knockback_,
    },
  };

  base->param = (typeof(base->param)) {0};
}

void lochara_base_deinitialize(lochara_base_t* base) {
  assert(base != NULL);
  assert(!base->used);

}

void lochara_base_calculate_physics(
    lochara_base_t* base, const vec2_t* size, const vec2_t* offset) {
  assert(base != NULL);
  assert(vec2_valid(size));
  assert(vec2_valid(offset));

  const float dt = base->ticker->delta_f;

  vec2_t velocity = base->param.movement;
  velocity.y += base->param.gravity;
  velocity.x += base->param.knockback;

  base->param.gravity -= GRAVITY_ACCELERATION_*dt;
  locommon_easing_linear_float(
      &base->param.knockback, 0, KNOCKBACK_RECOVERY_ACCELERATION_*dt);

  vec2_t disp;
  vec2_mul(&disp, &velocity, base->ticker->delta_f);

  vec2_addeq(&base->super.super.pos.fract, &disp);
  vec2_subeq(&base->super.super.pos.fract, offset);
  locommon_position_reduce(&base->super.super.pos);

  locommon_physics_entity_t e = {
    .size     = *size,
    .pos      = base->super.super.pos,
    .velocity = velocity,
  };

  loentity_store_solve_collision_between_ground(
      base->entities, &e, base->ticker->delta_f);

  base->super.super.pos = e.pos;
  vec2_addeq(&base->super.super.pos.fract, offset);
  locommon_position_reduce(&base->super.super.pos);

  base->param.on_ground = false;
  if (e.velocity.y == 0) {
    if (velocity.y <= 0) {
      base->param.on_ground = true;
    }
    if (base->param.gravity*velocity.y > 0) {
      base->param.gravity = 0;
    }
  }
  if (e.velocity.x == 0 && velocity.x != 0) {
    if (base->param.knockback*velocity.x >= 0) {
      base->param.knockback = 0;
    }
  }
  base->super.velocity = velocity = e.velocity;
}

void lochara_base_bind_on_ground(lochara_base_t* base, const vec2_t* offset) {
  assert(base != NULL);
  assert(vec2_valid(offset));

  if (base->cache.ground == NULL) return;

  vec2_t p;
  locommon_position_sub(
      &p, &base->super.super.pos, &base->cache.ground->super.pos);

  const vec2_t sz = base->cache.ground->size;
  p.x = MATH_CLAMP(p.x, -sz.x+offset->x, sz.x-offset->x);
  p.y = MATH_CLAMP(p.y,  sz.y+offset->y, 1);

  base->super.super.pos = base->cache.ground->super.pos;
  vec2_addeq(&base->super.super.pos.fract, &p);
  locommon_position_reduce(&base->super.super.pos);
}

bool lochara_base_affect_bullets(lochara_base_t* base) {
  assert(base != NULL);

  const uint64_t t = base->ticker->time;
  if (base->param.last_bullet_hit + BULLET_INVINCIBLE_DURATION_ > t) {
    return false;
  }

  const bool hit = loentity_store_affect_bullets_shot_by_others(
      base->entities,
      &base->super,
      &base->super.velocity,
      base->ticker->delta_f);
  if (hit) {
    base->param.last_bullet_hit = base->ticker->time;
  }
  return true;
}

bool lochara_base_unpack(lochara_base_t* base, const msgpack_object* obj) {
  assert(base != NULL);
  assert(obj  != NULL);
  assert(!base->used);

  lochara_base_reinitialize(base, 0);  /* id will be overwritten */

  loeffect_recipient_initialize(&base->param.recipient, base->ticker, NULL);

  const msgpack_object_map* root = mpkutil_get_map(obj);
  if (root == NULL) goto FAIL;
  CRIAL_DESERIALIZER_;
  return true;

FAIL:
  lochara_base_delete_(&base->super.super);
  return false;
}
