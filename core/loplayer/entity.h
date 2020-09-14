#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/locommon/ticker.h"
#include "core/loentity/character.h"
#include "core/loentity/store.h"
#include "core/loresource/sound.h"
#include "core/loshader/character.h"

#include "./event.h"
#include "./status.h"

typedef struct {
  loentity_character_t super;

  /* injected deps */
  const locommon_ticker_t*     ticker;
  loresource_sound_t*          sound;
  loshader_character_drawer_t* drawer;
  loentity_store_t*            entities;
  const loplayer_event_t*      event;
  loplayer_status_t*           status;

  /* read-only mutable params */
  float direction;

  /* public params */
  vec2_t movement;
  vec2_t knockback;
  float  gravity;

  /* temporary cache for update */
  bool   on_ground;
  vec2_t last_velocity;

  /* temporary cache for draw */
  struct {
    loshader_character_motion_id_t from;
    loshader_character_motion_id_t to;
    float                          time;
  } motion;
} loplayer_entity_t;

void
loplayer_entity_initialize(
    loplayer_entity_t*           player,
    loentity_id_t                id,
    loresource_sound_t*          sound,
    loshader_character_drawer_t* drawer,
    const locommon_ticker_t*     ticker,
    loentity_store_t*            entities,
    const loplayer_event_t*      event,
    loplayer_status_t*           status
);

void
loplayer_entity_deinitialize(
    loplayer_entity_t* player
);

void
loplayer_entity_move(
    loplayer_entity_t*         player,
    const locommon_position_t* pos
);

void
loplayer_entity_aim(
    loplayer_entity_t*         player,
    const locommon_position_t* pos
);

bool
loplayer_entity_affect_bullet(
    loplayer_entity_t* player
);

void
loplayer_entity_pack(
    const loplayer_entity_t* player,
    msgpack_packer*          packer
);

bool
loplayer_entity_unpack(
    loplayer_entity_t*    player,
    const msgpack_object* obj
);
