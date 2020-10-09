#include "./player.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/statman/statman.h"

#include "core/lobullet/linear.h"
#include "core/lobullet/pool.h"
#include "core/locommon/easing.h"
#include "core/locommon/position.h"
#include "core/loeffect/effect.h"
#include "core/loeffect/recipient.h"
#include "core/loentity/store.h"
#include "core/loplayer/stance.h"
#include "core/loshader/character.h"

#include "./base.h"
#include "./state.h"
#include "./type.h"

#define INITIAL_POS_ locommon_position(0, 0, vec2(.5f, .8f))

#define WIDTH_  .02f
#define HEIGHT_ .05f
#define MARKER_ .03f
#define COLOR_  vec4(0, 0, 0, 1)

#define RESPAWN_DURATION_ 4000

#define BULLET_SIZE_      vec2(.02f, .02f)
#define BULLET_COLOR_     vec4(1, .9f, .9f, 1)
#define BULLET_KNOCKBACK_ 4
#define BULLET_DAMAGE_    .2f
#define BULLET_DURATION_  1000
#define BULLET_COST_      .06f

#define CAMERA_SPEED_   10
#define CAMERA_SHIFT_Y_ .1f

#define CAMERA_FADE_RADIAL_ .6f

#define CAMERA_DEAD_FADE_           2
#define CAMERA_DEAD_SPEED_          .3f
#define CAMERA_DEAD_RECOVERY_SPEED_ 10

#define CAMERA_COMBAT_SPEED_          10
#define CAMERA_COMBAT_RECOVERY_SPEED_ 5

#define CAMERA_ENEMY_ATTACK_INTENSITY_ .2f
#define CAMERA_ENEMY_ATTACK_SPEED_     4

#define CAMERA_DAMAGE_INTENSITY_ .5f
#define CAMERA_DAMAGE_DURATION_  1000

#define CAMERA_ABERRATION_ 1

#define CAMERA_CORRUPTION_THRESH_ .2f
#define CAMERA_CORRUPTION_PERIOD_ 4000

#define DIRECTION_X_EPSILON_ .005f

#define GUARD_THRESHOLD_     .8f
#define BACKGUARD_THRESHOLD_ .95f

static const loeffect_recipient_status_t base_status_ = {
  .attack  = .2f,
  .defence = .2f,
  .speed   = .3f,
  .jump    = 1.f,
};

static void lochara_player_update_dead_state_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  lochara_base_t* base = instance;
  base->param.movement = vec2(0, 0);

  const uint64_t t = base->ticker->time - base->param.last_state_changed;

  if (t >= RESPAWN_DURATION_ && base->player->event.executor == 0) {
    loeffect_recipient_apply_effect(
        &base->param.recipient, &loeffect_resuscitate());
    base->super.super.pos = base->param.anchor.pos;

    *next = LOCHARA_STATE_STAND;
    return;
  }
  base->cache.instance.motion.from = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  base->cache.instance.motion.time = 0;
}

static void lochara_player_update_shoot_state_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  static const uint64_t dur = 200;

  lochara_base_t* base = instance;

  const uint64_t t  = base->ticker->time - base->param.last_state_changed;
  const float    dt = base->ticker->delta_f;

  locommon_easing_linear_float(&base->param.movement.x, 0, dt);
  locommon_easing_linear_float(&base->param.movement.y, 0, dt);

  const bool has_stance = loplayer_stance_set_has(
      &base->player->stances, LOPLAYER_STANCE_REVOLUTIONER);

  if (has_stance && t >= dur && base->param.recipient.faith > 0) {
    locommon_position_t pos = base->super.super.pos;
    pos.fract.x += WIDTH_*MATH_SIGN_NOZERO(base->param.direction.x);
    locommon_position_reduce(&pos);

    lobullet_base_t* b = lobullet_pool_create(base->bullet);
    lobullet_linear_circle_build(b,
        .owner     = base->super.super.id,
        .basepos   = pos,
        .size      = BULLET_SIZE_,
        .color     = BULLET_COLOR_,
        .velocity  = base->param.direction,
        .knockback = BULLET_KNOCKBACK_,
        .effect    = loeffect_damage(
            base->param.recipient.status.attack*BULLET_DAMAGE_),
        .duration  = BULLET_DURATION_,);
    loentity_store_add(base->entities, &b->super.super);

    loentity_character_apply_effect(
        &base->super, &loeffect_lost(BULLET_COST_));
    base->param.last_state_changed = base->ticker->time;
  }

  base->cache.instance.motion.from = LOSHADER_CHARACTER_MOTION_ID_STAND1;
  base->cache.instance.motion.to   = LOSHADER_CHARACTER_MOTION_ID_STAND2;
  base->cache.instance.motion.time = t > dur? 1: t*1.f / dur;
}

static const statman_meta_t state_table_[] = {
  lochara_state_stand(
        .period       = 2000,
        .acceleration = {{5, 5}},
        .motion1      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
        .motion2      = LOSHADER_CHARACTER_MOTION_ID_STAND2,
      ),
  lochara_state_walk(
        .period       = 350,
        .acceleration = {{5, 5}},
        .motion1      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
        .motion2      = LOSHADER_CHARACTER_MOTION_ID_WALK,
      ),
  lochara_state_sprint(
        .period       = 300,
        .acceleration = {{5, 5}},
        .speed        = 1.4f,
        .motion1      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
        .motion2      = LOSHADER_CHARACTER_MOTION_ID_WALK,
      ),
  lochara_state_dodge(
        .duration     = 100,
        .acceleration = {{3, 3}},
        .speed        = 1,
        .motion1      = LOSHADER_CHARACTER_MOTION_ID_WALK,
        .motion2      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
      ),
  lochara_state_jump(),

  {
    .state = LOCHARA_STATE_GUARD,
    .name  = "GUARD",
    .data  = &(lochara_state_move_param_t) {
      .speed        = 0,
      .period       = 1000,
      .acceleration = {{10, 10}},
      .motion1      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
      .motion2      = LOSHADER_CHARACTER_MOTION_ID_STAND1,
    },
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_state_update_move_,
  },
  {
    .state = LOCHARA_STATE_SHOOT,
    .name  = "SHOOT",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_player_update_shoot_state_,
  },
  {
    .state = LOCHARA_STATE_DEAD,
    .name  = "DEAD",
    .initialize = lochara_state_initialize_any_,
    .update     = lochara_player_update_dead_state_,
    .finalize   = lochara_state_cancel_transition_,
  },
  {0},
};

static void lochara_player_handle_controller_(lochara_base_t* base) {
  assert(base != NULL);

  lochara_state_t next = base->param.state;
  switch (base->player->controller.state) {
  case LOPLAYER_CONTROLLER_STATE_NONE:
    next = LOCHARA_STATE_STAND;
    break;
  case LOPLAYER_CONTROLLER_STATE_WALK_LEFT:
    next = LOCHARA_STATE_WALK_LEFT;
    break;
  case LOPLAYER_CONTROLLER_STATE_WALK_RIGHT:
    next = LOCHARA_STATE_WALK_RIGHT;
    break;
  case LOPLAYER_CONTROLLER_STATE_SPRINT_LEFT:
    next = LOCHARA_STATE_SPRINT_LEFT;
    break;
  case LOPLAYER_CONTROLLER_STATE_SPRINT_RIGHT:
    next = LOCHARA_STATE_SPRINT_RIGHT;
    break;
  case LOPLAYER_CONTROLLER_STATE_DODGE_FORWARD:
    next =
        base->param.direction.x < 0?
        LOCHARA_STATE_DODGE_LEFT:
        LOCHARA_STATE_DODGE_RIGHT;
    break;
  case LOPLAYER_CONTROLLER_STATE_DODGE_LEFT:
    next = LOCHARA_STATE_DODGE_LEFT;
    break;
  case LOPLAYER_CONTROLLER_STATE_DODGE_RIGHT:
    next = LOCHARA_STATE_DODGE_RIGHT;
    break;
  case LOPLAYER_CONTROLLER_STATE_JUMP:
    if (base->param.on_ground) next = LOCHARA_STATE_JUMP;
    break;
  case LOPLAYER_CONTROLLER_STATE_GUARD:
    next = LOCHARA_STATE_GUARD;
    break;
  case LOPLAYER_CONTROLLER_STATE_SHOOT:
    next = LOCHARA_STATE_SHOOT;
    break;
  }

  if (loplayer_combat_is_attack_pending(&base->player->combat)) {
    if (next != LOCHARA_STATE_STAND       &&
        next != LOCHARA_STATE_DODGE_LEFT  &&
        next != LOCHARA_STATE_DODGE_RIGHT &&
        next != LOCHARA_STATE_GUARD) {
      next = LOCHARA_STATE_STAND;
    }
  } else {
    vec2_t dir;
    locommon_position_sub(
        &dir, &base->player->controller.cursor, &base->super.super.pos);
    if (fabsf(dir.x) > DIRECTION_X_EPSILON_) {
      vec2_div(&base->param.direction, &dir, vec2_length(&dir));
    }
  }
  statman_transition_to(state_table_, base, &base->param.state, next);
}

static void lochara_player_update_combat_(lochara_base_t* base) {
  assert(base != NULL);

  const lochara_state_t state = base->param.state;

  if (state == LOCHARA_STATE_DEAD) {
    loplayer_combat_set_accepting(
        &base->player->combat,
        false,
        LOPLAYER_COMBAT_ATTACK_RESULT_ABORTED);
  } else {
    loplayer_combat_set_accepting(
        &base->player->combat,
        state != LOCHARA_STATE_DODGE_LEFT &&
        state != LOCHARA_STATE_DODGE_RIGHT,
        LOPLAYER_COMBAT_ATTACK_RESULT_DODGED);
  }

  loplayer_combat_set_guarding(
      &base->player->combat,
      state == LOCHARA_STATE_GUARD);

  if (loplayer_combat_is_attack_pending(&base->player->combat)) {
    base->param.movement = vec2(0, 0);
    base->param.gravity  = 0;
  }

  float r;
  loplayer_combat_attack_t a;
  if (!loplayer_combat_pop_attack(&base->player->combat, &a, &r)) return;

  loentity_store_iterator_t itr;
  if (!loentity_store_find_item_by_id(base->entities, &itr, a.attacker) ||
      itr.character == NULL) {
    return;
  }
  vec2_t disp;
  locommon_position_sub(
      &disp, &itr.character->super.pos, &base->super.super.pos);

  const float dir       = disp.x * base->param.direction.x;
  const bool  reflected =
      r > (dir < 0? BACKGUARD_THRESHOLD_: GUARD_THRESHOLD_);

  loplayer_combat_attack_handle(
      &a,
      reflected?
        LOPLAYER_COMBAT_ATTACK_RESULT_REFLECTED:
        LOPLAYER_COMBAT_ATTACK_RESULT_EXECUTED);
  if (!reflected) return;

  loresource_sound_set_play(&base->res->sound, LORESOURCE_SOUND_ID_REFLECTION);

  loentity_character_apply_effect(
      itr.character,
      &loeffect_damage(base->param.recipient.status.attack));

  loentity_character_knockback(
      itr.character,
      &vec2(MATH_SIGN(disp.x)*base->param.recipient.status.attack, 0));
}

static void lochara_player_update_camera_(lochara_base_t* base) {
  assert(base != NULL);

  const loeffect_recipient_t* r  = &base->param.recipient;
  const uint64_t              t  = base->ticker->time;
  const uint64_t              pt = base->ticker->prev_time;
  const float                 dt = base->ticker->delta_f;

  const bool combat = loplayer_combat_is_attack_pending(&base->player->combat);

  loplayer_camera_t* camera = &base->player->camera;

  vec2_t sz = base->player->camera.chunk_winsz;
  vec2_diveq(&sz,
      base->player->camera.scale*
      (camera->posteffect.distortion_radial+1));

  locommon_position_t pos = base->super.super.pos;
  if (!combat) {
    pos.fract.y += CAMERA_SHIFT_Y_;
    locommon_position_reduce(&pos);
  }

  loplayer_event_bind_rect_in_area(&base->player->event, &pos, &sz);

  vec2_t disp;
  locommon_position_sub(&disp, &pos, &camera->pos);
  if (vec2_pow_length(&disp) < 1) {
    locommon_easing_smooth_position(&camera->pos, &pos, dt*CAMERA_SPEED_);
  } else {
    camera->pos = pos;
  }
  locommon_easing_smooth_float(&camera->scale, 1, dt);

  /* ---- dead ---- */
  if (base->param.state == LOCHARA_STATE_DEAD) {
    locommon_easing_smooth_float(
        &camera->posteffect.fade_radial,
        CAMERA_DEAD_FADE_,
        dt*CAMERA_DEAD_SPEED_);
  } else {
    locommon_easing_smooth_float(
        &camera->posteffect.fade_radial,
        CAMERA_FADE_RADIAL_,
        dt*CAMERA_DEAD_RECOVERY_SPEED_);
  }

  /* ---- combat ---- */
  if (combat) {
    locommon_easing_smooth_float(
        &camera->posteffect.distortion_radial,
        1,
        dt*CAMERA_COMBAT_SPEED_);
  } else {
    locommon_easing_smooth_float(
        &camera->posteffect.distortion_radial,
        0,
        dt*CAMERA_COMBAT_RECOVERY_SPEED_);
  }

  /* ---- enemy attack ---- */
  const loplayer_combat_attack_t* attack = base->player->combat.first_attack;
  if (attack != NULL && pt < attack->start && attack->start <= t) {
    camera->posteffect.distortion_urgent = CAMERA_ENEMY_ATTACK_INTENSITY_;
  } else {
    locommon_easing_smooth_float(
        &camera->posteffect.distortion_urgent,
        0,
        dt*CAMERA_ENEMY_ATTACK_SPEED_);
  }

  /* ---- damage ----- */
  if (0 < r->last_damage &&
        r->last_damage < t && t < r->last_damage+CAMERA_DAMAGE_DURATION_) {
    camera->posteffect.raster_whole =
        (1 - (t - r->last_damage)*1.f/CAMERA_DAMAGE_DURATION_)*
        CAMERA_DAMAGE_INTENSITY_;
  } else {
    camera->posteffect.raster_whole = 0;
  }

  /* ---- amnesia ---- */
  locommon_easing_smooth_float(
      &camera->posteffect.distortion_amnesia,
      !!(r->effects.amnesia.start < t &&
        t < r->effects.amnesia.start+r->effects.amnesia.duration),
      dt);

  /* ---- corruption ---- */
  if (r->madness <= CAMERA_CORRUPTION_THRESH_) {
    if (camera->corruption_since == 0) camera->corruption_since = t;
    const uint64_t p =
        (t - camera->corruption_since)%CAMERA_CORRUPTION_PERIOD_;
    camera->pixsort = 1-powf(p*1.f/CAMERA_CORRUPTION_PERIOD_, 2);
  } else {
    camera->corruption_since = 0;
    locommon_easing_smooth_float(&camera->pixsort, 0, dt);
  }

  /* ---- passive ---- */
  locommon_easing_smooth_float(
      &camera->posteffect.aberration_radial, CAMERA_ABERRATION_, dt);
}

bool lochara_player_update(lochara_base_t* base) {
  assert(base != NULL);

  loeffect_recipient_update(&base->param.recipient, &base_status_);
  if (!loeffect_recipient_is_alive(&base->param.recipient)) {
    statman_transition_to(
        state_table_, base, &base->param.state, LOCHARA_STATE_DEAD);
  }
  if (loplayer_stance_set_has(
        &base->player->stances, LOPLAYER_STANCE_UNFINISHER)) {
    if (base->param.recipient.faith > .5f) {
      loeffect_recipient_apply_effect(
          &base->param.recipient, &loeffect_heal(base->ticker->delta_f*.01f));
    }
  }

  lochara_player_handle_controller_(base);

  const float dir = MATH_SIGN_NOZERO(base->param.direction.x);
  base->cache.instance = (loshader_character_drawer_instance_t) {
    .character_id  = LOSHADER_CHARACTER_ID_PLAYER,
    .pos           = vec2(0, -MARKER_),
    .size          = vec2(dir*HEIGHT_, HEIGHT_),
    .color         = COLOR_,
    .marker_offset = vec2(0, MARKER_),
  };

  statman_update(state_table_, base, &base->param.state);

  lochara_player_update_combat_(base);

  if (base->param.state != LOCHARA_STATE_DODGE_LEFT &&
      base->param.state != LOCHARA_STATE_DODGE_RIGHT &&
      base->param.state != LOCHARA_STATE_DEAD) {
    base->cache.instance.marker = lochara_base_affect_bullets(base);
  }

  lochara_base_calculate_physics(
      base, &vec2(WIDTH_, HEIGHT_), &vec2(0, MARKER_));
  loplayer_event_bind_rect_in_area(
      &base->player->event,
      &base->super.super.pos,
      &vec2(WIDTH_*.8f, HEIGHT_));

  lochara_player_update_camera_(base);
  return true;
}

void lochara_player_build(lochara_base_t* base) {
  assert(base != NULL);

  base->super.super.pos = base->param.anchor.pos = INITIAL_POS_;

  base->param = (typeof(base->param)) {
    .type               = LOCHARA_TYPE_PLAYER,
    .state              = LOCHARA_STATE_STAND,
    .last_state_changed = base->ticker->time,
  };
  loeffect_recipient_initialize(
      &base->param.recipient, base->ticker, &base_status_);
}
