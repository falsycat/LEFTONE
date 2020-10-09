#include "./event.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/msgpack.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"

/* generated serializer */
#include "core/loplayer/crial/event.h"

static bool loplayer_event_execute_command_(
    loplayer_event_t* ev, const loplayer_event_command_t* command) {
  assert(ev != NULL);

  if (ev->exectime > ev->ticker->time) return false;

  switch (command->type) {
  case LOPLAYER_EVENT_COMMAND_TYPE_NONE:
    return false;
  case LOPLAYER_EVENT_COMMAND_TYPE_FINALIZE:
    ev->basetime = 0;
    ev->executor = 0;
    ev->commands = NULL;
    ev->ctx      = (typeof(ev->ctx)) {
      .line = { .last_update = ev->ticker->time, },
    };
    return true;
  case LOPLAYER_EVENT_COMMAND_TYPE_PLAY_MUSIC:
    ev->ctx.music = (typeof(ev->ctx.music)) {
      .enable = true,
      .id     = command->music,
      .since  = ev->exectime,
    };
    return true;
  case LOPLAYER_EVENT_COMMAND_TYPE_STOP_MUSIC:
    ev->ctx.music = (typeof(ev->ctx.music)) {
      .enable = false,
    };
    return true;
  case LOPLAYER_EVENT_COMMAND_TYPE_SET_AREA:
    ev->ctx.area.size = command->area;
    return true;
  case LOPLAYER_EVENT_COMMAND_TYPE_SET_CINESCOPE:
    ev->ctx.cinescope = command->cinescope;
    return true;
  case LOPLAYER_EVENT_COMMAND_TYPE_SET_LINE:
    if (ev->ctx.line.text_id != command->line) {
      ev->ctx.line.text_id     = command->line;
      ev->ctx.line.last_update = ev->ticker->time;
    }
    return true;
  case LOPLAYER_EVENT_COMMAND_TYPE_WAIT:
    ev->exectime += command->wait;
    return true;
  }
  __builtin_unreachable();
}

void loplayer_event_initialize(
    loplayer_event_t* ev, const locommon_ticker_t* ticker) {
  assert(ev     != NULL);
  assert(ticker != NULL);

  *ev = (typeof(*ev)) {
    .ticker = ticker,
  };
}

void loplayer_event_deinitialize(loplayer_event_t* ev) {
  assert(ev != NULL);

}

bool loplayer_event_execute_commands(
    loplayer_event_t*               ev,
    loentity_id_t                   executor,
    const locommon_position_t*      pos,
    const loplayer_event_command_t* commands) {
  assert(ev       != NULL);
  assert(commands != NULL);
  assert(locommon_position_valid(pos));

  const bool first = ev->basetime == 0;

  if (first) ev->executor = executor;
  if (ev->executor != executor) return false;

  if (ev->commands != commands) {
    if (ev->commands != NULL || first) {
      ev->basetime = ev->ticker->time;
    }
    ev->commands = commands;
    ev->itr      = commands;
    ev->exectime = ev->basetime;
  }
  ev->ctx.area.pos = *pos;

  while (loplayer_event_execute_command_(ev, ev->itr)) ++ev->itr;
  return true;
}

void loplayer_event_pack(const loplayer_event_t* ev, msgpack_packer* packer) {
  assert(ev     != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, CRIAL_PROPERTY_COUNT_);
  CRIAL_SERIALIZER_;
}

bool loplayer_event_unpack(loplayer_event_t* ev, const msgpack_object* obj) {
  assert(ev  != NULL);
  assert(obj != NULL);

  const msgpack_object_map* root = mpkutil_get_map(obj);
  CRIAL_DESERIALIZER_;
  return true;
}

void loplayer_event_bind_position_in_area(
    const loplayer_event_t* ev, locommon_position_t* pos) {
  assert(ev != NULL);
  assert(locommon_position_valid(pos));

  loplayer_event_bind_rect_in_area(ev, pos, &vec2(0, 0));
}

void loplayer_event_bind_rect_in_area(
    const loplayer_event_t* ev, locommon_position_t* pos, const vec2_t* size) {
  assert(ev != NULL);
  assert(locommon_position_valid(pos));
  assert(vec2_valid(size));

  if (ev->ctx.area.size.x <= 0 || ev->ctx.area.size.y <= 0) return;

  vec2_t sz;
  vec2_sub(&sz, &ev->ctx.area.size, size);
  if (sz.x < 0) sz.x = 0;
  if (sz.y < 0) sz.y = 0;

  vec2_t v;
  locommon_position_sub(&v, pos, &ev->ctx.area.pos);

  if (fabs(v.x) > sz.x) v.x = MATH_SIGN(v.x)*sz.x;
  if (fabs(v.y) > sz.y) v.y = MATH_SIGN(v.y)*sz.y;

  *pos = ev->ctx.area.pos;
  vec2_addeq(&pos->fract, &v);
  locommon_position_reduce(pos);
}
