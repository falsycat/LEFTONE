#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/locommon/input.h"
#include "core/locommon/position.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loentity/store.h"

#include "./camera.h"
#include "./combat.h"
#include "./controller.h"
#include "./event.h"
#include "./popup.h"
#include "./stance.h"

typedef struct lochara_base_t lochara_base_t;

typedef struct {
  loplayer_stance_set_t stances;

  loplayer_combat_t     combat;
  loplayer_controller_t controller;
  loplayer_camera_t     camera;
  loplayer_event_t      event;
  loplayer_popup_t      popup;

  lochara_base_t* entity;
} loplayer_t;

void
loplayer_initialize(
    loplayer_t*              player,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities
);

void
loplayer_deinitialize(
    loplayer_t* player
);

void
loplayer_update(
    loplayer_t*                player,
    const locommon_input_t*    input,  /* NULLABLE */
    const locommon_position_t* cursor  /* NULLABLE */
);

void
loplayer_pack(
    const loplayer_t* player,
    msgpack_packer*   packer
);

/* WARNING: the entity must be unpacked manually */
bool
loplayer_unpack(
    loplayer_t*           player,
    const msgpack_object* obj  /* NULLABLE */
);
