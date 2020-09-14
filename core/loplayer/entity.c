#include "./entity.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include <msgpack.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/easing.h"
#include "core/locommon/msgpack.h"
#include "core/locommon/physics.h"
#include "core/locommon/ticker.h"
#include "core/loentity/character.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"
#include "core/loresource/sound.h"
#include "core/loshader/character.h"

#include "./event.h"
#include "./status.h"

#define LOPLAYER_ENTITY_WIDTH     .02f
#define LOPLAYER_ENTITY_HEIGHT    .05f
#define LOPLAYER_ENTITY_DRAW_SIZE LOPLAYER_ENTITY_HEIGHT
#define LOPLAYER_ENTITY_SHIFT_Y   .03f

#define LOPLAYER_ENTITY_GRAVITY_ACCELARATION  2.2f
#define LOPLAYER_ENTITY_RECOVERY_ACCELARATION 1.f

#define LOPLAYER_ENTITY_MAX_GRAVITY 2.f

#define LOPLAYER_ENTITY_DIRECTION_EPSILON .05f

#define LOPLAYER_ENTITY_PARAM_TO_PACK_EACH_(PROC) do {  \
  PROC("pos",       super.super.pos);  \
  PROC("movement",  movement);  \
  PROC("knockback", knockback);  \
  PROC("gravity",   gravity);  \
} while (0)
#define LOPLAYER_ENTITY_PARAM_TO_PACK_COUNT 4

static void loplayer_entity_update_position_(
    loplayer_entity_t* p, vec2_t* velocity) {
  assert(p != NULL);
  assert(vec2_valid(velocity));

  vec2_t disp = *velocity;
  vec2_muleq(&disp, p->ticker->delta_f);

  vec2_addeq(&p->super.super.pos.fract, &disp);
  p->super.super.pos.fract.y -= LOPLAYER_ENTITY_SHIFT_Y;
  locommon_position_reduce(&p->super.super.pos);

  locommon_physics_entity_t e = {
    .size     = vec2(LOPLAYER_ENTITY_WIDTH, LOPLAYER_ENTITY_HEIGHT),
    .pos      = p->super.super.pos,
    .velocity = *velocity,
  };

  loentity_store_solve_collision_between_ground(
      p->entities, &e, p->ticker->delta_f);

  p->super.super.pos = e.pos;

  p->super.super.pos.fract.y += LOPLAYER_ENTITY_SHIFT_Y;
  locommon_position_reduce(&p->super.super.pos);

  p->on_ground = false;
  if (e.velocity.y == 0) {
    if (velocity->y <= 0) {
      p->on_ground = true;
    }
    if (p->gravity*velocity->y     >= 0) p->gravity     = 0;
    if (p->knockback.y*velocity->y >= 0) p->knockback.y = 0;
  }
  if (e.velocity.x == 0 && velocity->x != 0) {
    if (p->knockback.x*velocity->x >= 0) p->knockback.x = 0;
  }
  p->last_velocity = *velocity = e.velocity;
}

static void loplayer_entity_bind_in_event_area_(loplayer_entity_t* p) {
  assert(p != NULL);

  const loplayer_event_param_t* e = loplayer_event_get_param(p->event);
  if (e == NULL || e->area_size.x <= 0 || e->area_size.y <= 0) return;

  vec2_t v;
  locommon_position_sub(&v, &p->super.super.pos, &e->area_pos);

  if (MATH_ABS(v.x) > e->area_size.x) {
    v.x = MATH_SIGN(v.x) * e->area_size.x;
  }
  if (MATH_ABS(v.y) > e->area_size.y) {
    v.y = MATH_SIGN(v.y) * e->area_size.y;
  }

  p->super.super.pos = e->area_pos;
  vec2_addeq(&p->super.super.pos.fract, &v);
  locommon_position_reduce(&p->super.super.pos);
}

static void loplayer_entity_delete_(loentity_t* entity) {
  assert(entity != NULL);

  /* does not anything */
}

static bool loplayer_entity_update_(loentity_t* entity) {
  assert(entity != NULL);

  loplayer_entity_t* p = (typeof(p)) entity;

  /* ---- position ---- */
  vec2_t velocity = p->movement;
  vec2_addeq(&velocity, &p->knockback);
  velocity.y += p->gravity;
  loplayer_entity_update_position_(p, &velocity);
  loplayer_entity_bind_in_event_area_(p);

  /* ---- gravity ---- */
  const float dt = p->ticker->delta_f;
  p->gravity -= LOPLAYER_ENTITY_GRAVITY_ACCELARATION*dt;
  p->gravity  = MATH_MAX(p->gravity, -LOPLAYER_ENTITY_MAX_GRAVITY);

  /* ---- recovery from knockback ---- */
  locommon_easing_linear_float(
      &p->knockback.x, 0, LOPLAYER_ENTITY_RECOVERY_ACCELARATION*dt);
  locommon_easing_linear_float(
      &p->knockback.y, 0, LOPLAYER_ENTITY_RECOVERY_ACCELARATION*dt);
  return true;
}

static void loplayer_entity_draw_(
    loentity_t* entity, const locommon_position_t* basepos) {
  assert(entity  != NULL);
  assert(basepos != NULL);

  loplayer_entity_t* p = (typeof(p)) entity;

  locommon_position_t center = p->super.super.pos;
  center.fract.y -= LOPLAYER_ENTITY_SHIFT_Y;
  locommon_position_reduce(&center);

  loshader_character_drawer_instance_t instance = {
    .character_id   = LOSHADER_CHARACTER_ID_PLAYER,
    .from_motion_id = p->motion.from,
    .to_motion_id   = p->motion.to,
    .motion_time    = p->motion.time,
    .marker         = p->status->bullet_immune_until < p->ticker->time,
    .marker_offset  = vec2(0, LOPLAYER_ENTITY_SHIFT_Y),

    .size  = vec2(
        LOPLAYER_ENTITY_DRAW_SIZE*p->direction, LOPLAYER_ENTITY_DRAW_SIZE),
    .color = vec4(0, 0, 0, 1),
  };
  locommon_position_sub(&instance.pos, &center, basepos);

  loshader_character_drawer_add_instance(p->drawer, &instance);
}

static void loplayer_entity_apply_effect_(
    loentity_character_t* chara, const loeffect_t* effect) {
  assert(chara  != NULL);
  assert(effect != NULL);

  loplayer_entity_t* p = (typeof(p)) chara;
  loplayer_status_apply_effect(p->status, effect);
}

static void loplayer_entity_knockback_(
    loentity_character_t* chara, const vec2_t* v) {
  assert(chara != NULL);
  assert(vec2_valid(v));

  loplayer_entity_t* p = (typeof(p)) chara;
  vec2_addeq(&p->knockback, v);
}

void loplayer_entity_initialize(
    loplayer_entity_t*           player,
    loentity_id_t                id,
    loresource_sound_t*          sound,
    loshader_character_drawer_t* drawer,
    const locommon_ticker_t*     ticker,
    loentity_store_t*            entities,
    const loplayer_event_t*      event,
    loplayer_status_t*           status) {
  assert(player   != NULL);
  assert(sound    != NULL);
  assert(drawer   != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);
  assert(event    != NULL);
  assert(status   != NULL);

  *player = (typeof(*player)) {
    .super = {
      .super = {
        .vtable = {
          .delete = loplayer_entity_delete_,
          .update = loplayer_entity_update_,
          .draw   = loplayer_entity_draw_,
        },
        .subclass  = LOENTITY_SUBCLASS_CHARACTER,
        .id        = id,
        .pos       = locommon_position(0, 0, vec2(0.5, 0.5)),
        .dont_save = true,
      },
      .vtable = {
        .apply_effect = loplayer_entity_apply_effect_,
        .knockback    = loplayer_entity_knockback_,
      },
    },
    .sound    = sound,
    .drawer   = drawer,
    .ticker   = ticker,
    .entities = entities,
    .event    = event,
    .status   = status,

    .direction = 1,
  };
}

void loplayer_entity_deinitialize(loplayer_entity_t* player) {
  assert(player != NULL);

}

void loplayer_entity_move(
    loplayer_entity_t* player, const locommon_position_t* pos) {
  assert(player != NULL);
  assert(locommon_position_valid(pos));

  player->super.super.pos = *pos;
  player->on_ground       = false;
  player->movement        = vec2(0, 0);
  player->knockback       = vec2(0, 0);
  player->gravity         = 0;
}

void loplayer_entity_aim(
    loplayer_entity_t* player, const locommon_position_t* pos) {
  assert(player != NULL);
  assert(locommon_position_valid(pos));

  vec2_t dir;
  locommon_position_sub(&dir, pos, &player->super.super.pos);

  if (MATH_ABS(dir.x) > LOPLAYER_ENTITY_DIRECTION_EPSILON) {
    player->direction = MATH_SIGN(dir.x);
  }
}

bool loplayer_entity_affect_bullet(loplayer_entity_t* player) {
  assert(player != NULL);

  return loentity_store_affect_bullets_shot_by_others(
      player->entities,
      &player->super,
      &player->last_velocity,
      player->ticker->delta_f);
}

void loplayer_entity_pack(
    const loplayer_entity_t* player, msgpack_packer* packer) {
  assert(player != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, LOPLAYER_ENTITY_PARAM_TO_PACK_COUNT);

# define pack_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &player->var);  \
  } while (0)

  LOPLAYER_ENTITY_PARAM_TO_PACK_EACH_(pack_);

# undef pack_
}

bool loplayer_entity_unpack(
    loplayer_entity_t* player, const msgpack_object* obj) {
  assert(player != NULL);

  if (obj == NULL) return false;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define unpack_(name, var) do {  \
    if (!LOCOMMON_MSGPACK_UNPACK_ANY(item_(name), &player->var)) {  \
      return false;  \
    }  \
  } while (0)

  LOPLAYER_ENTITY_PARAM_TO_PACK_EACH_(unpack_);
  return true;

# undef unpack_
# undef item_
}
