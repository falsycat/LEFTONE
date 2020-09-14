#include "./status.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/easing.h"
#include "core/locommon/msgpack.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/recipient.h"
#include "core/loeffect/stance.h"
#include "core/loresource/set.h"
#include "core/loresource/sound.h"

static const loeffect_recipient_status_t loplayer_status_base_ = {
  .attack  = .3f,
  .defence = .2f,
  .speed   = .3f,
  .jump    = 1.f,
};

#define LOPLAYER_STATUS_PARAM_TO_PACK_EACH_(PROC) do {  \
  PROC("stances",             stances);  \
  PROC("madness",             recipient.madness);  \
  PROC("faith",               recipient.faith);  \
  PROC("effects",             recipient.effects);  \
  PROC("respawn-pos",         respawn_pos);  \
  PROC("last-damage-time",    last_damage_time);  \
  PROC("bullet-immune-until", bullet_immune_until);  \
} while (0)
#define LOPLAYER_STATUS_PARAM_TO_PACK_COUNT 7

#define LOPLAYER_STATUS_MADNESS_DYING_THRESHOLD .3f

void loplayer_status_initialize(
    loplayer_status_t*       status,
    loresource_set_t*        res,
    const locommon_ticker_t* ticker) {
  assert(status != NULL);
  assert(ticker != NULL);

  *status = (typeof(*status)) {
    .res    = res,
    .ticker = ticker,

    .respawn_pos = locommon_position(0, 0, vec2(.5f, .5f)),
  };

  loeffect_stance_set_initialize(&status->stances);

  loeffect_recipient_initialize(&status->recipient, ticker);
}

void loplayer_status_deinitialize(loplayer_status_t* status) {
  assert(status != NULL);

  loeffect_stance_set_deinitialize(&status->stances);
  loeffect_recipient_deinitialize(&status->recipient);
}

void loplayer_status_reset(loplayer_status_t* status) {
  assert(status != NULL);

  loeffect_recipient_reset(&status->recipient);
}

bool loplayer_status_add_stance(
    loplayer_status_t* status, loeffect_stance_id_t id) {
  assert(status != NULL);

  if (loeffect_stance_set_has(&status->stances, id)) return false;
  loeffect_stance_set_add(&status->stances, id);
  return true;
}

void loplayer_status_remove_stance(
    loplayer_status_t* status, loeffect_stance_id_t id) {
  assert(status != NULL);

  loeffect_stance_set_remove(&status->stances, id);
}

void loplayer_status_apply_effect(
    loplayer_status_t* status, const loeffect_t* effect) {
  assert(status != NULL);
  assert(effect != NULL);

  if (effect->id == LOEFFECT_ID_IMMEDIATE_DAMAGE) {
    status->last_damage_time = status->ticker->time;
    loresource_sound_play(status->res->sound, "damage");
  }

  loeffect_recipient_apply_effect(&status->recipient, effect);
}

void loplayer_status_update(loplayer_status_t* status) {
  assert(status != NULL);

  static const uint64_t dying_sound_period = 4000;

  loeffect_recipient_status_t base = loplayer_status_base_;
  if (loeffect_stance_set_has(&status->stances, LOEFFECT_STANCE_ID_PHILOSOPHER)) {
    base.defence += .3f;
  }

  loeffect_recipient_update(&status->recipient, &base);

  /* Makes decreasing madness faster if the player has a stance, UNFINISHER. */
  if (loeffect_stance_set_has(
        &status->stances, LOEFFECT_STANCE_ID_UNFINISHER)) {
    if (status->recipient.madness > 0) {
      if (status->recipient.faith <= 0) {
        status->recipient.madness -= status->ticker->delta_f/30;
        status->recipient.last_damage = LOEFFECT_ID_LOST;
      } else if (status->recipient.faith >= .5f) {
        status->recipient.madness += status->ticker->delta_f/120;
      }
    }
  }

  if (status->recipient.madness > 0 &&
      status->recipient.madness <= LOPLAYER_STATUS_MADNESS_DYING_THRESHOLD) {
    uint64_t t  = status->ticker->time;
    uint64_t pt = t - status->ticker->delta;

    status->dying_effect = t%dying_sound_period*1.f / dying_sound_period;

    t  /= dying_sound_period;
    pt /= dying_sound_period;

    if (t != pt) loresource_sound_play(status->res->sound, "dying");
  } else {
    locommon_easing_linear_float(
        &status->dying_effect, 0, status->ticker->delta_f*5);
  }
}

void loplayer_status_set_respawn_position(
    loplayer_status_t* status, const locommon_position_t* pos) {
  assert(status != NULL);
  assert(locommon_position_valid(pos));

  status->respawn_pos = *pos;
}

void loplayer_status_pack(
    const loplayer_status_t* status, msgpack_packer* packer) {
  assert(status != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, LOPLAYER_STATUS_PARAM_TO_PACK_COUNT);

# define pack_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &status->var);  \
  } while (0)

  LOPLAYER_STATUS_PARAM_TO_PACK_EACH_(pack_);

# undef pack_
}

bool loplayer_status_unpack(
    loplayer_status_t* status, const msgpack_object* obj) {
  assert(status != NULL);

  if (obj == NULL) return false;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define unpack_(name, var) do {  \
    if (!LOCOMMON_MSGPACK_UNPACK_ANY(item_(name), &status->var)) {  \
      return false;  \
    }  \
  } while (0)

  LOPLAYER_STATUS_PARAM_TO_PACK_EACH_(unpack_);
  return true;

# undef unpack_
# undef item_
}
