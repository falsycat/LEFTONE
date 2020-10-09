#include "./state.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/statman/statman.h"

#include "core/locommon/easing.h"
#include "core/loresource/sound.h"
#include "core/loshader/character.h"

#include "./base.h"

#define MIDAIR_SPEED_   .8f
#define BACKWALK_SPEED_ .6f

void lochara_state_initialize_any_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  lochara_base_t* base = instance;
  base->param.last_state_changed = base->ticker->time;
}
void lochara_state_cancel_transition_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  *state = meta->state;
}

void lochara_state_update_move_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  const lochara_state_move_param_t* p    = meta->data;
  lochara_base_t*                   base = instance;

  const uint64_t t  = base->ticker->time - base->param.last_state_changed;
  const float    dt = base->ticker->delta_f;

  assert(vec2_valid(&p->acceleration));
  assert(MATH_FLOAT_VALID(p->speed));
  assert(vec2_valid(&p->velocity));
  assert(p->period > 0);

  /* ---- acceleration ---- */
  float speed = p->speed*base->param.recipient.status.speed;
  if (!base->param.on_ground) {
    speed *= MIDAIR_SPEED_;
  }
  if (vec2_dot(&p->velocity, &base->param.direction) < 0) {
    speed *= BACKWALK_SPEED_;
  }

  vec2_t velocity = p->velocity;
  vec2_muleq(&velocity, speed);

  locommon_easing_linear_float(
      &base->param.movement.x, velocity.x, p->acceleration.x*dt);
  locommon_easing_linear_float(
      &base->param.movement.y, velocity.y, p->acceleration.y*dt);

  /* ---- periodic motion ---- */
  base->cache.instance.motion.time =
      (!!base->param.on_ground)*(1-fabs(t%p->period*1.f/p->period*2 - 1));
  base->cache.instance.motion.from = p->motion1;
  base->cache.instance.motion.to   = p->motion2;
}

void lochara_state_initialize_dodge_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  lochara_state_initialize_any_(meta, instance, state);

  lochara_base_t* base = instance;
  loresource_sound_set_play(&base->res->sound, LORESOURCE_SOUND_ID_DODGE);
}

void lochara_state_update_dodge_(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(state    != NULL);

  const lochara_state_dodge_param_t* p    = meta->data;
  lochara_base_t*                    base = instance;

  const uint64_t t = base->ticker->time - base->param.last_state_changed;

  assert(p->duration > 0);
  assert(vec2_valid(&p->acceleration));
  assert(vec2_valid(&p->velocity));

  if (t > p->duration) {
    *state = LOCHARA_STATE_STAND;
    return;
  }

  /* ---- acceleration ---- */
  base->param.movement = p->acceleration;

  vec2_muleq(&base->param.movement, t/1000.f);
  base->param.movement.x *= -MATH_SIGN(p->velocity.x);
  base->param.movement.y *= -MATH_SIGN(p->velocity.y);

  vec2_addeq(&base->param.movement, &p->velocity);

  /* ---- motion ---- */
  base->cache.instance.motion.time = powf(t*1.f/p->duration, 1.2f);
  base->cache.instance.motion.from = p->motion1;
  base->cache.instance.motion.to   = p->motion2;

  const float a = powf(t*2.f/p->duration-1, 2);
  base->cache.instance.size.x  *= (1-a)*.8f+1;
  base->cache.instance.color.w *= MATH_MIN(a+.2f, 1);
}

void lochara_state_initialize_jump_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  lochara_state_initialize_any_(meta, instance, next);

  lochara_base_t* base = instance;
  base->param.gravity += base->param.recipient.status.jump;
  *next = LOCHARA_STATE_STAND;
}

void lochara_state_update_teleport_(
    const statman_meta_t* meta, void* instance, statman_state_t* next) {
  assert(meta     != NULL);
  assert(instance != NULL);
  assert(next     != NULL);

  const lochara_state_teleport_param_t* p    = meta->data;
  lochara_base_t*                       base = instance;

  base->param.movement = vec2(0, 0);

  const uint64_t t  = base->ticker->time - base->param.last_state_changed;
  const uint64_t pt =
      t == 0? 0: base->ticker->prev_time - base->param.last_state_changed;

  if (pt < p->duration/2 && p->duration/2 <= t) {
    const lochara_base_t* player = base->player->entity;

    const float pdir =
        MATH_SIGN_NOZERO(player->param.direction.x)*p->direction;
    base->param.direction = vec2(-pdir, 0);

    vec2_t offset = p->offset;
    offset.x *= pdir;

    base->super.super.pos = player->super.super.pos;
    vec2_addeq(&base->super.super.pos.fract, &offset);
    locommon_position_reduce(&base->super.super.pos);
  }

  const float tf = fabsf(t*1.f/p->duration*2 - 1);
  base->cache.instance.motion.from = p->motion2;
  base->cache.instance.motion.to   = p->motion1;
  base->cache.instance.motion.time = tf;
  base->cache.instance.color.w     = tf;
}
