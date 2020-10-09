#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loresource/music.h"

typedef enum {
  LOPLAYER_EVENT_COMMAND_TYPE_NONE,
  LOPLAYER_EVENT_COMMAND_TYPE_FINALIZE,

  LOPLAYER_EVENT_COMMAND_TYPE_PLAY_MUSIC,
  LOPLAYER_EVENT_COMMAND_TYPE_STOP_MUSIC,

  LOPLAYER_EVENT_COMMAND_TYPE_SET_AREA,
  LOPLAYER_EVENT_COMMAND_TYPE_SET_CINESCOPE,
  LOPLAYER_EVENT_COMMAND_TYPE_SET_LINE,

  LOPLAYER_EVENT_COMMAND_TYPE_WAIT,
} loplayer_event_command_type_t;

typedef struct {
  loplayer_event_command_type_t type;
  union {
    loresource_music_id_t music;

    vec2_t area;

    float cinescope;

    const char* line;

    uint64_t wait;
  };
} loplayer_event_command_t;

#define loplayer_event_command_finalize()  \
    (loplayer_event_command_t) {  \
      .type = LOPLAYER_EVENT_COMMAND_TYPE_FINALIZE,  \
    }
#define loplayer_event_command_play_music(id)  \
    (loplayer_event_command_t) {  \
      .type  = LOPLAYER_EVENT_COMMAND_TYPE_PLAY_MUSIC,  \
      .music = id,  \
    }
#define loplayer_event_command_stop_music()  \
    (loplayer_event_command_t) {  \
      .type  = LOPLAYER_EVENT_COMMAND_TYPE_STOP_MUSIC,  \
    }
#define loplayer_event_command_set_area(w, h)  \
    (loplayer_event_command_t) {  \
      .type = LOPLAYER_EVENT_COMMAND_TYPE_SET_AREA,  \
      .area = {{w, h}},  \
    }
#define loplayer_event_command_set_cinescope(v)  \
    (loplayer_event_command_t) {  \
      .type      = LOPLAYER_EVENT_COMMAND_TYPE_SET_CINESCOPE,  \
      .cinescope = v,  \
    }
#define loplayer_event_command_set_line(text_id)  \
    (loplayer_event_command_t) {  \
      .type = LOPLAYER_EVENT_COMMAND_TYPE_SET_LINE,  \
      .line = text_id,  \
    }
#define loplayer_event_command_wait(dur)  \
    (loplayer_event_command_t) {  \
      .type = LOPLAYER_EVENT_COMMAND_TYPE_WAIT,  \
      .wait = dur,  \
    }

typedef struct {
  struct {
    locommon_position_t pos;
    vec2_t              size;
  } area;

  float cinescope;

  struct {
    const char* text_id;
    uint64_t    last_update;
  } line;

  struct {
    bool                  enable;
    loresource_music_id_t id;
    uint64_t              since;
  } music;
} loplayer_event_context_t;

typedef struct {
  const locommon_ticker_t* ticker;

  loentity_id_t executor;
  uint64_t      basetime;

  const loplayer_event_command_t* commands;
  const loplayer_event_command_t* itr;
  uint64_t                        exectime;

  loplayer_event_context_t ctx;
} loplayer_event_t;

void
loplayer_event_initialize(
    loplayer_event_t*        ev,
    const locommon_ticker_t* ticker
);

void
loplayer_event_deinitialize(
    loplayer_event_t* ev
);

bool
loplayer_event_execute_commands(
    loplayer_event_t*               ev,
    loentity_id_t                   executor,
    const locommon_position_t*      pos,
    const loplayer_event_command_t* commands  /* ends with NONE */
);

void
loplayer_event_pack(
    const loplayer_event_t* ev,
    msgpack_packer*         packer
);

bool
loplayer_event_unpack(
    loplayer_event_t*     ev,
    const msgpack_object* obj
);

void
loplayer_event_bind_position_in_area(
    const loplayer_event_t* ev,
    locommon_position_t*    pos
);

void
loplayer_event_bind_rect_in_area(
    const loplayer_event_t* ev,
    locommon_position_t*    pos,
    const vec2_t*           size
);
