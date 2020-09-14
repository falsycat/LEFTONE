#include "./action.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/algorithm.h"
#include "util/memory/memory.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/lobullet/base.h"
#include "core/lobullet/linear.h"
#include "core/lobullet/pool.h"
#include "core/locommon/easing.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/effect.h"
#include "core/loeffect/stance.h"
#include "core/loentity/store.h"
#include "core/loresource/set.h"
#include "core/loresource/sound.h"
#include "core/loresource/text.h"

#include "./camera.h"
#include "./combat.h"
#include "./controller.h"
#include "./entity.h"
#include "./event.h"
#include "./hud.h"
#include "./menu.h"
#include "./status.h"

struct loplayer_action_t {
  loresource_set_t*        res;
  const locommon_ticker_t* ticker;
  lobullet_pool_t*         bullets;
  loentity_store_t*        entities;

  loplayer_event_t*            event;
  loplayer_status_t*           status;
  loplayer_entity_t*           entity;
  const loplayer_controller_t* controller;
  loplayer_combat_t*           combat;
  loplayer_camera_t*           camera;
  loplayer_hud_t*              hud;
  loplayer_menu_t*             menu;

  union {
    struct {
    } stand;
    struct {
      float direction;
    } moving;
    struct {
      float direction;
    } dodge;
    struct {
    } combat;
    struct {
    } shoot;
    struct {
    } dead;
    struct {
      bool invincible;
    } menu;
  } state;
  uint64_t since;

  void
  (*execute)(
      loplayer_action_t* action
  );
  void
  (*pack)(
      const loplayer_action_t* action,
      msgpack_packer*          packer
  );
};

#define LOPLAYER_ACTION_STATE_EACH_(PROC) do {  \
  PROC(stand);  \
  PROC(moving);  \
  PROC(dodge);  \
  PROC(combat);  \
  PROC(shoot);  \
  PROC(dead);  \
  PROC(menu);  \
} while (0)

static void
loplayer_action_start_stand_state_(
    loplayer_action_t* action
);
static void
loplayer_action_start_moving_state_(
    loplayer_action_t* action,
    float              direction
);
static void
loplayer_action_start_dodge_state_(
    loplayer_action_t* action,
    float              direction
);
static void
loplayer_action_start_combat_state_(
    loplayer_action_t* action
);
static bool
loplayer_action_start_shoot_state_(
    loplayer_action_t* action
);
static void
loplayer_action_start_dead_state_(
    loplayer_action_t* action
);
static void
loplayer_action_start_menu_state_(
    loplayer_action_t* action,
    bool               invincible
);

static void loplayer_action_affect_bullet_(loplayer_action_t* action) {
  assert(action != NULL);

  if (action->status->bullet_immune_until > action->ticker->time) {
    return;
  }
  if (loplayer_entity_affect_bullet(action->entity)) {
    action->status->bullet_immune_until = action->ticker->time + 200;
  }
}
static bool loplayer_action_shoot_bullet_(loplayer_action_t* action) {
  assert(action != NULL);

  static const float consume = .05f;

  float* f = &action->status->recipient.faith;
  if (*f <= 0) return false;

  vec2_t v;
  locommon_position_sub(
      &v, &action->controller->looking, &action->entity->super.super.pos);
  const float vlen = vec2_length(&v);
  if (vlen == 0) {
    v = vec2(action->entity->direction, 0);
  } else {
    vec2_diveq(&v, vec2_length(&v));
  }

  /* TODO(catfoot): diffusion */
  vec2_muleq(&v, 1.f);
  vec2_addeq(&v, &action->entity->last_velocity);

  lobullet_base_t* b = lobullet_pool_create(action->bullets);
  lobullet_linear_light_build(b, (&(lobullet_linear_param_t) {
        .owner        = action->entity->super.super.id,
        .pos          = action->entity->super.super.pos,
        .size         = vec2(.015f, .015f),
        .velocity     = v,
        .acceleration = vec2(0, -.1f),
        .color        = vec4(.8f, .8f, .8f, .8f),
        .duration     = 2000,
        .knockback    = .1f,
        .effect       = loeffect_immediate_damage(
            action->status->recipient.status.attack/2),
      }));
  loentity_store_add(action->entities, &b->super.super);

  *f -= consume;
  if (*f < 0) *f = 0;
  return true;
}

static void loplayer_action_show_tutorial_after_death_(loplayer_action_t* action) {
  assert(action != NULL);

# define text_(name) loresource_text_get(action->res->lang, name)
# define popup_(name)  \
      loplayer_menu_popup(  \
          action->menu,  \
          text_("tutorial_title_"name),  \
          text_("tutorial_text_" name))

  switch (action->status->recipient.last_damage) {
  case LOEFFECT_ID_IMMEDIATE_DAMAGE:
    popup_("dead_by_combat");
    break;
  case LOEFFECT_ID_CURSE:
    popup_("dead_by_curse");
    break;
  case LOEFFECT_ID_LOST:
    popup_("dead_by_lost");
    break;
  default:
    return;
  }

# undef popup_
# undef text_

  loplayer_action_start_menu_popup_state(action);
}

static void loplayer_action_execute_stand_state_(loplayer_action_t* action) {
  assert(action != NULL);

  const float max_acceleration_ = action->entity->on_ground? 2.0f: 0.5f;

  loplayer_entity_aim(action->entity, &action->controller->looking);

  if (action->status->recipient.madness <= 0) {
    loplayer_action_start_dead_state_(action);
    return;
  }

  loplayer_action_affect_bullet_(action);
  if (loplayer_combat_accept_all_attacks(action->combat)) {
    loplayer_action_start_combat_state_(action);
    return;
  }

  switch (action->controller->action) {
  case LOPLAYER_CONTROLLER_ACTION_NONE:
    break;
  case LOPLAYER_CONTROLLER_ACTION_ATTACK:
    if (loplayer_action_start_shoot_state_(action)) return;
    break;
  case LOPLAYER_CONTROLLER_ACTION_DODGE:
    loplayer_action_start_dodge_state_(action, action->entity->direction);
    return;
  case LOPLAYER_CONTROLLER_ACTION_GUARD:
    break;
  case LOPLAYER_CONTROLLER_ACTION_UNGUARD:
    break;
  case LOPLAYER_CONTROLLER_ACTION_MENU:
    if (action->entity->movement.x == 0) {
      loplayer_menu_show_status(action->menu);
      loplayer_action_start_menu_state_(action, false  /* INVINCIBLE */);
      return;
    }
    break;
  }

  switch (action->controller->movement) {
  case LOPLAYER_CONTROLLER_MOVEMENT_NONE:
    break;
  case LOPLAYER_CONTROLLER_MOVEMENT_JUMP:
    if (action->entity->on_ground) {
      action->entity->gravity += action->status->recipient.status.jump;
    }
    break;
  case LOPLAYER_CONTROLLER_MOVEMENT_WALK_LEFT:
  case LOPLAYER_CONTROLLER_MOVEMENT_DASH_LEFT:
    loplayer_action_start_moving_state_(action, -1);
    return;
  case LOPLAYER_CONTROLLER_MOVEMENT_WALK_RIGHT:
  case LOPLAYER_CONTROLLER_MOVEMENT_DASH_RIGHT:
    loplayer_action_start_moving_state_(action, 1);
    return;
  }

  const float t = (action->ticker->time - action->since)%2000/1000.0f - 1;
  action->entity->motion.time = t*t*(3-2*MATH_ABS(t));
  action->entity->motion.from = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  action->entity->motion.to   = LOSHADER_CHARACTER_MOTION_ID_STAND2;

  locommon_easing_linear_float(
      &action->entity->movement.x,
      0,
      max_acceleration_ * action->ticker->delta_f);
}
static void loplayer_action_pack_stand_state_(
    const loplayer_action_t* action, msgpack_packer* packer) {
  assert(action != NULL);

  msgpack_pack_map(packer, 1);

  mpkutil_pack_str(packer, "name");
  mpkutil_pack_str(packer, "stand");
}
static bool loplayer_action_unpack_stand_state_(
    loplayer_action_t* action, const msgpack_object_map* root) {
  assert(action != NULL);

  return root != NULL;
}
static void loplayer_action_start_stand_state_(loplayer_action_t* action) {
  assert(action != NULL);

  action->since   = action->ticker->time;
  action->execute = loplayer_action_execute_stand_state_;
  action->pack    = loplayer_action_pack_stand_state_;

  action->camera->state = LOPLAYER_CAMERA_STATE_DEFAULT;
  loplayer_hud_show(action->hud);
}

static void loplayer_action_execute_moving_state_(loplayer_action_t* action) {
  assert(action != NULL);

  static const float backwalk_attenuation_ = 0.8f;
  static const float dash_speed_           = 1.4f;

  const float max_acceleration_ = action->entity->on_ground? 2.4f: 0.8f;

  const float dir = action->state.moving.direction;

  loplayer_entity_aim(action->entity, &action->controller->looking);

  if (action->status->recipient.madness <= 0) {
    loplayer_action_start_dead_state_(action);
    return;
  }

  loplayer_action_affect_bullet_(action);
  if (loplayer_combat_accept_all_attacks(action->combat)) {
    loplayer_action_start_combat_state_(action);
    return;
  }

  switch (action->controller->action) {
  case LOPLAYER_CONTROLLER_ACTION_NONE:
    break;
  case LOPLAYER_CONTROLLER_ACTION_ATTACK:
    if (loplayer_action_start_shoot_state_(action)) return;
    break;
  case LOPLAYER_CONTROLLER_ACTION_DODGE:
    loplayer_action_start_dodge_state_(action, dir);
    return;
  case LOPLAYER_CONTROLLER_ACTION_GUARD:
    break;
  case LOPLAYER_CONTROLLER_ACTION_UNGUARD:
    break;
  case LOPLAYER_CONTROLLER_ACTION_MENU:
    break;
  }

  float max_speed   = action->status->recipient.status.speed;
  float control_dir = dir;

  switch (action->controller->movement) {
  case LOPLAYER_CONTROLLER_MOVEMENT_NONE:
    loplayer_action_start_stand_state_(action);
    return;
  case LOPLAYER_CONTROLLER_MOVEMENT_JUMP:
    if (action->entity->on_ground) {
      action->entity->gravity += action->status->recipient.status.jump;
    }
    return;
  case LOPLAYER_CONTROLLER_MOVEMENT_WALK_LEFT:
    control_dir = -1;
    break;
  case LOPLAYER_CONTROLLER_MOVEMENT_WALK_RIGHT:
    control_dir = 1;
    break;
  case LOPLAYER_CONTROLLER_MOVEMENT_DASH_LEFT:
    max_speed   *= dash_speed_;
    control_dir = -1;
    break;
  case LOPLAYER_CONTROLLER_MOVEMENT_DASH_RIGHT:
    max_speed   *= dash_speed_;
    control_dir = 1;
    break;
  }
  if (control_dir * dir < 0) {
    loplayer_action_start_stand_state_(action);
    return;
  }

  if (dir * action->entity->direction < 0) {
    max_speed *= backwalk_attenuation_;
  }

  if (action->entity->on_ground) {
    const int32_t p = 70/max_speed;
    const float   t = (action->ticker->time - action->since)%p*2.0f/p - 1;
    action->entity->motion.time = MATH_ABS(t);
    action->entity->motion.from = LOSHADER_CHARACTER_MOTION_ID_STAND1;
    action->entity->motion.to   = LOSHADER_CHARACTER_MOTION_ID_WALK;
  }

  locommon_easing_linear_float(
      &action->entity->movement.x,
      max_speed*dir,
      max_acceleration_ * action->ticker->delta_f);
}
static void loplayer_action_pack_moving_state_(
    const loplayer_action_t* action, msgpack_packer* packer) {
  assert(action != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 2);

  mpkutil_pack_str(packer, "name");
  mpkutil_pack_str(packer, "moving");

  mpkutil_pack_str(packer, "direction");
  msgpack_pack_double(packer, action->state.moving.direction);
}
static bool loplayer_action_unpack_moving_state_(
    loplayer_action_t* action, const msgpack_object_map* root) {
  assert(action != NULL);

  const msgpack_object* direction =
      mpkutil_get_map_item_by_str(root, "direction");
  if (!mpkutil_get_float(direction, &action->state.moving.direction)) {
    return false;
  }
  return true;
}
static void loplayer_action_start_moving_state_(
    loplayer_action_t* action, float dir) {
  assert(action != NULL);
  assert(MATH_FLOAT_VALID(dir));

  action->since   = action->ticker->time;
  action->execute = loplayer_action_execute_moving_state_;
  action->pack    = loplayer_action_pack_moving_state_;

  action->state = (typeof(action->state)) {
    .moving = {
      .direction = dir,
    },
  };
  action->camera->state = LOPLAYER_CAMERA_STATE_DEFAULT;
  loplayer_hud_show(action->hud);
}

static void loplayer_action_execute_dodge_state_(loplayer_action_t* action) {
  assert(action != NULL);

  static const uint64_t duration_    = 200;
  static const float    start_speed_ = 0.6f;
  static const float    end_speed_   = 0.1f;

  if (action->since + duration_ <= action->ticker->time) {
    loplayer_combat_drop_all_attacks(action->combat);
    loplayer_action_start_stand_state_(action);
    return;
  }

  const float dir = action->state.dodge.direction;

  vec2_t* v = &action->entity->movement;

  const float t = (action->ticker->time - action->since)*1.0f/duration_;
  const float r = 1 - powf(1-t, 1.5);
  v->x = (r * (start_speed_-end_speed_) + end_speed_) * dir;
  v->y = 0;

  action->entity->motion.time = 1-powf(1-t, 2);
  action->entity->motion.from = LOSHADER_CHARACTER_MOTION_ID_WALK;
  action->entity->motion.to   = LOSHADER_CHARACTER_MOTION_ID_STAND1;
}
static void loplayer_action_pack_dodge_state_(
    const loplayer_action_t* action, msgpack_packer* packer) {
  assert(action != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 2);

  mpkutil_pack_str(packer, "name");
  mpkutil_pack_str(packer, "dodge");

  mpkutil_pack_str(packer, "direction");
  msgpack_pack_double(packer, action->state.dodge.direction);
}
static bool loplayer_action_unpack_dodge_state_(
    loplayer_action_t* action, const msgpack_object_map* root) {
  assert(action != NULL);

  const msgpack_object* direction =
      mpkutil_get_map_item_by_str(root, "direction");
  if (!mpkutil_get_float(direction, &action->state.moving.direction)) {
    return false;
  }
  return true;
}
static void loplayer_action_start_dodge_state_(
    loplayer_action_t* action, float dir) {
  assert(action != NULL);
  assert(MATH_FLOAT_VALID(dir));

  action->since = action->ticker->time;
  action->state = (typeof(action->state)) {
    .moving = {
      .direction = dir,
    },
  };
  action->execute = loplayer_action_execute_dodge_state_;
  action->pack    = loplayer_action_pack_dodge_state_;

  action->camera->state = LOPLAYER_CAMERA_STATE_DEFAULT;
  loplayer_hud_show(action->hud);
  loplayer_combat_drop_all_attacks(action->combat);

  loresource_sound_play(action->res->sound, "dodge");
}

static void loplayer_action_execute_combat_state_(loplayer_action_t* action) {
  assert(action != NULL);

  if (action->status->recipient.madness <= 0) {
    loplayer_action_start_dead_state_(action);
    return;
  }

  loplayer_action_affect_bullet_(action);
  if (!loplayer_combat_accept_all_attacks(action->combat)) {
    loplayer_action_start_stand_state_(action);
    return;
  }

  switch (action->controller->action) {
  case LOPLAYER_CONTROLLER_ACTION_NONE:
    break;
  case LOPLAYER_CONTROLLER_ACTION_ATTACK:
    break;
  case LOPLAYER_CONTROLLER_ACTION_DODGE:
    loplayer_action_start_dodge_state_(action, action->entity->direction);
    return;
  case LOPLAYER_CONTROLLER_ACTION_GUARD:
    loplayer_combat_guard(action->combat);
    break;
  case LOPLAYER_CONTROLLER_ACTION_UNGUARD:
    loplayer_combat_unguard(action->combat);
    break;
  case LOPLAYER_CONTROLLER_ACTION_MENU:
    break;
  }

  action->entity->gravity = 0;

  const float klen = vec2_length(&action->entity->knockback);
  if (klen > .1f) vec2_muleq(&action->entity->knockback, .1f/klen);
}
static void loplayer_action_pack_combat_state_(
    const loplayer_action_t* action, msgpack_packer* packer) {
  assert(action != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 1);

  mpkutil_pack_str(packer, "name");
  mpkutil_pack_str(packer, "combat");
}
static bool loplayer_action_unpack_combat_state_(
    loplayer_action_t* action, const msgpack_object_map* root) {
  assert(action != NULL);

  return root != NULL;
}
static void loplayer_action_start_combat_state_(loplayer_action_t* action) {
  assert(action != NULL);

  action->since   = action->ticker->time;
  action->execute = loplayer_action_execute_combat_state_;
  action->pack    = loplayer_action_pack_combat_state_;

  action->entity->movement = vec2(0, 0);

  action->camera->state = LOPLAYER_CAMERA_STATE_COMBAT;
  loplayer_hud_show(action->hud);
}

static void loplayer_action_execute_shoot_state_(loplayer_action_t* action) {
  assert(action != NULL);

  static const uint64_t duration = 300;
  static const float    max_acceleration = 1.f;

  loplayer_entity_aim(action->entity, &action->controller->looking);

  if (action->status->recipient.madness <= 0) {
    loplayer_action_start_dead_state_(action);
    return;
  }

  loplayer_action_affect_bullet_(action);
  if (loplayer_combat_accept_all_attacks(action->combat)) {
    loplayer_action_start_combat_state_(action);
    return;
  }

  if (action->since+duration <= action->ticker->time) {
    if (loplayer_action_shoot_bullet_(action)) {
      loresource_sound_play(action->res->sound, "player_shoot");
    }
    loplayer_action_start_stand_state_(action);
    return;
  }

  const float a = max_acceleration * action->ticker->delta_f;
  locommon_easing_linear_float(&action->entity->movement.x, 0, a);
  locommon_easing_linear_float(&action->entity->movement.y, 0, a);
}
static void loplayer_action_pack_shoot_state_(
    const loplayer_action_t* action, msgpack_packer* packer) {
  assert(action != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 1);

  mpkutil_pack_str(packer, "name");
  mpkutil_pack_str(packer, "shoot");
}
static bool loplayer_action_unpack_shoot_state_(
    loplayer_action_t* action, const msgpack_object_map* root) {
  assert(action != NULL);

  return root != NULL;
}
static bool loplayer_action_start_shoot_state_(loplayer_action_t* action) {
  assert(action != NULL);

  if (!loeffect_stance_set_has(
        &action->status->stances, LOEFFECT_STANCE_ID_REVOLUTIONER)) {
    return false;
  }

  action->since   = action->ticker->time;
  action->execute = loplayer_action_execute_shoot_state_;
  action->pack    = loplayer_action_pack_shoot_state_;

  action->camera->state = LOPLAYER_CAMERA_STATE_DEFAULT;
  loplayer_hud_show(action->hud);

  loresource_sound_play(action->res->sound, "player_trigger");
  return true;
}

static void loplayer_action_execute_dead_state_(loplayer_action_t* action) {
  assert(action != NULL);

  static const uint64_t duration_ = 3000;

  if (action->since + duration_ <= action->ticker->time) {
    loplayer_entity_move(action->entity, &action->status->respawn_pos);
    loplayer_status_reset(action->status);
    loplayer_combat_drop_all_attacks(action->combat);

    loplayer_action_start_stand_state_(action);
    loplayer_action_show_tutorial_after_death_(action);
    loplayer_event_abort(action->event);
    return;
  }
}
static void loplayer_action_pack_dead_state_(
    const loplayer_action_t* action, msgpack_packer* packer) {
  assert(action != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 1);

  mpkutil_pack_str(packer, "name");
  mpkutil_pack_str(packer, "dead");
}
static bool loplayer_action_unpack_dead_state_(
    loplayer_action_t* action, const msgpack_object_map* root) {
  assert(action != NULL);

  return root != NULL;
}
static void loplayer_action_start_dead_state_(loplayer_action_t* action) {
  assert(action != NULL);

  action->since   = action->ticker->time;
  action->execute = loplayer_action_execute_dead_state_;
  action->pack    = loplayer_action_pack_dead_state_;

  action->entity->movement = vec2(0, 0);

  action->camera->state = LOPLAYER_CAMERA_STATE_DEAD;
  loplayer_hud_hide(action->hud);
  loplayer_combat_drop_all_attacks(action->combat);

  /* Deny all event requests. */
  loplayer_event_abort(action->event);
  loplayer_event_take_control(action->event, action->entity->super.super.id);
}

static void loplayer_action_execute_menu_state_(loplayer_action_t* action) {
  assert(action != NULL);

  if (action->status->recipient.madness <= 0) {
    loplayer_menu_hide(action->menu);
    loplayer_action_start_dead_state_(action);
    return;
  }

  loplayer_action_affect_bullet_(action);
  if (!action->state.menu.invincible &&
        loplayer_combat_accept_all_attacks(action->combat)) {
    loplayer_menu_hide(action->menu);
    loplayer_action_start_combat_state_(action);
    return;
  }

  switch (action->controller->action) {
  case LOPLAYER_CONTROLLER_ACTION_NONE:
    break;
  case LOPLAYER_CONTROLLER_ACTION_ATTACK:
    break;
  case LOPLAYER_CONTROLLER_ACTION_DODGE:
    break;
  case LOPLAYER_CONTROLLER_ACTION_GUARD:
    break;
  case LOPLAYER_CONTROLLER_ACTION_UNGUARD:
    break;
  case LOPLAYER_CONTROLLER_ACTION_MENU:
    loplayer_menu_hide(action->menu);
    loplayer_action_start_stand_state_(action);
    return;
  }
}
static void loplayer_action_pack_menu_state_(
    const loplayer_action_t* action, msgpack_packer* packer) {
  assert(action != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 2);

  mpkutil_pack_str(packer, "name");
  mpkutil_pack_str(packer, "menu");

  mpkutil_pack_str(packer, "invincible");
  mpkutil_pack_bool(packer, action->state.menu.invincible);
}
static bool loplayer_action_unpack_menu_state_(
    loplayer_action_t* action, const msgpack_object_map* root) {
  assert(action != NULL);

  const msgpack_object* invincible =
      mpkutil_get_map_item_by_str(root, "invincible");
  if (!mpkutil_get_bool(invincible, &action->state.menu.invincible)) {
    return false;
  }
  return true;
}
static void loplayer_action_start_menu_state_(
    loplayer_action_t* action, bool invincible) {
  assert(action != NULL);

  action->since   = action->ticker->time;
  action->execute = loplayer_action_execute_menu_state_;
  action->pack    = loplayer_action_pack_menu_state_;

  action->entity->movement = vec2(0, 0);

  action->state.menu = (typeof(action->state.menu)) {
    .invincible = invincible,
  };

  action->camera->state = LOPLAYER_CAMERA_STATE_MENU;
}

loplayer_action_t* loplayer_action_new(
    loresource_set_t*            res,
    const locommon_ticker_t*     ticker,
    lobullet_pool_t*             bullets,
    loentity_store_t*            entities,
    loplayer_event_t*            event,
    loplayer_status_t*           status,
    loplayer_entity_t*           entity,
    loplayer_combat_t*           combat,
    const loplayer_controller_t* controller,
    loplayer_camera_t*           camera,
    loplayer_hud_t*              hud,
    loplayer_menu_t*             menu) {
  assert(res        != NULL);
  assert(ticker     != NULL);
  assert(bullets    != NULL);
  assert(entities   != NULL);
  assert(event      != NULL);
  assert(status     != NULL);
  assert(entity     != NULL);
  assert(combat     != NULL);
  assert(controller != NULL);
  assert(camera     != NULL);
  assert(hud        != NULL);
  assert(menu       != NULL);

  loplayer_action_t* action = memory_new(sizeof(*action));
  *action = (typeof(*action)) {
    .res        = res,
    .ticker     = ticker,
    .bullets    = bullets,
    .entities   = entities,
    .event      = event,
    .status     = status,
    .entity     = entity,
    .combat     = combat,
    .controller = controller,
    .camera     = camera,
    .hud        = hud,
    .menu       = menu,
  };
  loplayer_action_start_stand_state_(action);
  return action;
}

void loplayer_action_delete(loplayer_action_t* action) {
  if (action == NULL) return;

  memory_delete(action);
}

void loplayer_action_start_menu_popup_state(loplayer_action_t* action) {
  assert(action != NULL);

  loplayer_action_start_menu_state_(action, true  /* invincible */);
}

void loplayer_action_execute(loplayer_action_t* action) {
  assert(action != NULL);

  assert(action->execute != NULL);
  action->execute(action);
}

void loplayer_action_pack(const loplayer_action_t* action, msgpack_packer* packer) {
  assert(action != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 2);

  mpkutil_pack_str(packer, "since");
  msgpack_pack_uint64(packer, action->since);

  assert(action->pack != NULL);

  mpkutil_pack_str(packer, "state");
  action->pack(action, packer);
}

bool loplayer_action_unpack(
    loplayer_action_t* action, const msgpack_object* obj) {
  assert(action != NULL);

  if (obj == NULL) return false;

  const msgpack_object_map* root = mpkutil_get_map(obj);

  const msgpack_object* since = mpkutil_get_map_item_by_str(root, "since");
  if (!mpkutil_get_uint64(since, &action->since)) return false;

  const msgpack_object_map* state = mpkutil_get_map(
      mpkutil_get_map_item_by_str(root, "state"));

  bool state_loaded = false;

  const msgpack_object* name = mpkutil_get_map_item_by_str(state, "name");
  const char* v;
  size_t      len;
  if (!mpkutil_get_str(name, &v, &len)) {
    loplayer_action_start_stand_state_(action);
    state_loaded = true;
  }

# define unpack_state_(name_) do {  \
    if (!state_loaded && strncmp(v, #name_, len) == 0 && #name_[len] == 0) {  \
      action->execute = loplayer_action_execute_##name_##_state_;  \
      action->pack    = loplayer_action_pack_##name_##_state_;  \
      if (!loplayer_action_unpack_##name_##_state_(action, state)) {  \
        loplayer_action_start_stand_state_(action);  \
      }  \
      state_loaded = true;  \
    }  \
  } while (0)

  LOPLAYER_ACTION_STATE_EACH_(unpack_state_);

# undef unpack_state_

  return true;
}
