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

/* generated serializer */
#include "core/loeffect/crial/recipient.h"

#define LOST_DAMAGE_AMOUNT_ .03f
#define LOST_DAMAGE_PERIOD_ 1000

void loeffect_recipient_initialize(
    loeffect_recipient_t*              recipient,
    const locommon_ticker_t*           ticker,
    const loeffect_recipient_status_t* status) {
  assert(recipient != NULL);
  assert(ticker    != NULL);

  *recipient = (typeof(*recipient)) {
    .ticker  = ticker,
    .madness = 1,
    .faith   = 1,
  };
  if (status != NULL) recipient->status = *status;
}

void loeffect_recipient_deinitialize(loeffect_recipient_t* recipient) {
  assert(recipient != NULL);

}

void loeffect_recipient_apply_effect(
    loeffect_recipient_t* recipient, const loeffect_t* effect) {
  assert(recipient != NULL);
  assert(effect    != NULL);

  if (effect->id == LOEFFECT_ID_RESUSCITATE) {
    recipient->madness = 1;
    recipient->faith   = 1;
    recipient->effects = (typeof(recipient->effects)) {0};
    recipient->last_resuscitate = recipient->ticker->time;
  }
  if (!loeffect_recipient_is_alive(recipient)) return;

  const float faith_backup = recipient->faith;

  switch (effect->id) {
  case LOEFFECT_ID_NONE:
  case LOEFFECT_ID_RESUSCITATE:
    break;
  case LOEFFECT_ID_LOST_DAMAGE:
    recipient->madness -= LOST_DAMAGE_AMOUNT_;
    break;
  case LOEFFECT_ID_CURSE_TRIGGER:
    recipient->madness = 0;
    break;
  case LOEFFECT_ID_DAMAGE:
    recipient->madness -=
        effect->data.imm.amount * (1-recipient->status.defence);
    recipient->last_damage = recipient->ticker->time;
    break;
  case LOEFFECT_ID_HEAL:
    recipient->madness  += effect->data.imm.amount;
    recipient->last_heal = recipient->ticker->time;
    break;
  case LOEFFECT_ID_LOST:
    recipient->faith    -= effect->data.imm.amount;
    recipient->last_lost = recipient->ticker->time;
    break;
  case LOEFFECT_ID_RETRIEVAL:
    recipient->faith         += effect->data.imm.amount;
    recipient->last_retrieval = recipient->ticker->time;
    break;
  case LOEFFECT_ID_FANATIC:
    recipient->effects.fanatic       = effect->data.lasting;
    recipient->effects.fanatic.start = recipient->ticker->time;
    break;
  case LOEFFECT_ID_CURSE:
    recipient->effects.curse       = effect->data.lasting;
    recipient->effects.curse.start = recipient->ticker->time;
    break;
  case LOEFFECT_ID_AMNESIA:
    recipient->effects.amnesia       = effect->data.lasting;
    recipient->effects.amnesia.start = recipient->ticker->time;
    break;
  }

  if (!loeffect_recipient_is_alive(recipient)) {
    recipient->last_die        = recipient->ticker->time;
    recipient->last_die_reason = effect->id;
  }
  if (faith_backup > 0 && recipient->faith <= 0) {
    recipient->lost_damage_since = recipient->ticker->time;
  }

  recipient->madness = MATH_CLAMP(recipient->madness, 0, 1);
  recipient->faith   = MATH_CLAMP(recipient->faith,   0, 1);
}

void loeffect_recipient_update(
    loeffect_recipient_t* recipient, const loeffect_recipient_status_t* base) {
  assert(recipient != NULL);
  assert(base      != NULL);

  const uint64_t t  = recipient->ticker->time;
  const uint64_t pt = recipient->ticker->prev_time;

  recipient->status = *base;

  if (recipient->madness > 0 && recipient->faith <= 0) {
    const uint64_t since = recipient->lost_damage_since;
    if (pt < since ||
        (pt-since)/LOST_DAMAGE_PERIOD_ != (t-since)/LOST_DAMAGE_PERIOD_) {
      loeffect_recipient_apply_effect(recipient, &loeffect_lost_damage());
    }
  }

  const uint64_t fanatic_st = recipient->effects.fanatic.start;
  const uint64_t fanatic_ed = fanatic_st + recipient->effects.fanatic.duration;
  if (pt < fanatic_ed && fanatic_ed <= t && recipient->madness <= 0) {
    recipient->last_die        = recipient->ticker->time;
    recipient->last_die_reason = LOEFFECT_ID_FANATIC;
  }
}

bool loeffect_recipient_is_alive(const loeffect_recipient_t* recipient) {
  assert(recipient != NULL);

  if (recipient->madness > 0) return true;

  const uint64_t t = recipient->ticker->time;

  const uint64_t fanatic_st = recipient->effects.fanatic.start;
  const uint64_t fanatic_ed = fanatic_st + recipient->effects.fanatic.duration;
  if (fanatic_st <= t && t < fanatic_ed) return true;

  return false;
}

void loeffect_recipient_pack(
    const loeffect_recipient_t* recipient, msgpack_packer* packer) {
  assert(recipient != NULL);
  assert(packer    != NULL);

  msgpack_pack_map(packer, CRIAL_PROPERTY_COUNT_);
  CRIAL_SERIALIZER_;
}

bool loeffect_recipient_unpack(
    loeffect_recipient_t* recipient, const msgpack_object* obj) {
  assert(recipient != NULL);

  const msgpack_object_map* root = mpkutil_get_map(obj);
  if (root == NULL) return false;

  CRIAL_DESERIALIZER_;
  return true;
}
