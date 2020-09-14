#include "./recipient.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/algorithm.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/msgpack.h"
#include "core/locommon/ticker.h"

#include "./generic.h"

#define LOEFFECT_RECIPIENT_EFFECT_PARAM_EACH_(PROC) do {  \
  PROC(curse);  \
} while (0)
#define LOEFFECT_RECIPIENT_EFFECT_PARAM_COUNT 1

void loeffect_recipient_initialize(
    loeffect_recipient_t* recipient, const locommon_ticker_t* ticker) {
  assert(recipient != NULL);
  assert(ticker    != NULL);

  *recipient = (typeof(*recipient)) {
    .ticker  = ticker,
  };
  loeffect_recipient_reset(recipient);
}

void loeffect_recipient_deinitialize(loeffect_recipient_t* recipient) {
  assert(recipient != NULL);

}

void loeffect_recipient_reset(loeffect_recipient_t* recipient) {
  assert(recipient != NULL);

  recipient->madness = 1;
  recipient->faith   = 1;

  recipient->effects = (typeof(recipient->effects)) {0};
}

void loeffect_recipient_apply_effect(
    loeffect_recipient_t* recipient, const loeffect_t* effect) {
  assert(recipient != NULL);
  assert(effect    != NULL);

  if (recipient->madness <= 0) return;

  switch (effect->id) {
  case LOEFFECT_ID_IMMEDIATE_DAMAGE:
    recipient->madness -=
        effect->data.imm.amount * (1-recipient->status.defence);
    recipient->last_damage = LOEFFECT_ID_IMMEDIATE_DAMAGE;
    break;
  case LOEFFECT_ID_CURSE:
    recipient->effects.curse = effect->data.lasting;
    break;
  case LOEFFECT_ID_CURSE_TRIGGER:
    recipient->madness = 0;
    recipient->last_damage = LOEFFECT_ID_CURSE;
    break;
  case LOEFFECT_ID_AMNESIA:
    recipient->effects.amnesia = effect->data.lasting;
    break;
  default:
    ;
  }
}

void loeffect_recipient_update(
    loeffect_recipient_t* recipient, const loeffect_recipient_status_t* base) {
  assert(recipient != NULL);
  assert(base      != NULL);

  recipient->status = *base;

  if (recipient->madness > 0 && recipient->faith <= 0) {
    recipient->madness     -= recipient->ticker->delta_f / 30;
    recipient->last_damage  = LOEFFECT_ID_LOST;
  }

  recipient->madness = MATH_CLAMP(recipient->madness, 0, 1);
  recipient->faith   = MATH_CLAMP(recipient->faith,   0, 1);
}

void loeffect_recipient_effect_param_pack(
    const loeffect_recipient_effect_param_t* param,
    msgpack_packer*                          packer) {
  assert(param  != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, LOEFFECT_RECIPIENT_EFFECT_PARAM_COUNT);

# define each_(name) do {  \
    mpkutil_pack_str(packer, #name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &param->name);  \
  } while (0)

  LOEFFECT_RECIPIENT_EFFECT_PARAM_EACH_(each_);

# undef each_
}

bool loeffect_recipient_effect_param_unpack(
    loeffect_recipient_effect_param_t* param, const msgpack_object* obj) {
  assert(param != NULL);

  if (obj == NULL) return false;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define each_(name) do {  \
    if (!LOCOMMON_MSGPACK_UNPACK_ANY(item_(#name), &param->name)) {  \
      param->name = (typeof(param->name)) {0};  \
    }  \
  } while (0)

  LOEFFECT_RECIPIENT_EFFECT_PARAM_EACH_(each_);

# undef each_
# undef item_
  return true;
}
