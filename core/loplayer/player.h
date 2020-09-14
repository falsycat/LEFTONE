#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "util/math/matrix.h"

#include "core/lobullet/pool.h"
#include "core/locommon/input.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/stance.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./action.h"
#include "./camera.h"
#include "./combat.h"
#include "./controller.h"
#include "./entity.h"
#include "./event.h"
#include "./hud.h"
#include "./menu.h"
#include "./status.h"

typedef struct {
  loshader_set_t* shaders;

  loplayer_event_t* event;

  loplayer_status_t status;
  loplayer_entity_t entity;

  loplayer_combat_t* combat;

  loplayer_controller_t controller;

  loplayer_camera_t camera;
  loplayer_hud_t*   hud;
  loplayer_menu_t*  menu;

  loplayer_action_t* action;
} loplayer_t;

void
loplayer_initialize(
    loplayer_t*              player,
    loentity_id_t            id,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    lobullet_pool_t*         bullets,
    loentity_store_t*        entities,
    const mat4_t*            proj
);

void
loplayer_deinitialize(
    loplayer_t* player
);

void
loplayer_popup(
    loplayer_t* player,
    const char* title,
    const char* text
);

bool
loplayer_attack(
    loplayer_t*                     player,
    const loplayer_combat_attack_t* attack
);

void
loplayer_touch_encephalon(
    loplayer_t* player
);

void
loplayer_gain_stance(
    loplayer_t*          player,
    loeffect_stance_id_t id
);

void
loplayer_gain_faith(
    loplayer_t* player,
    float       amount
);

void
loplayer_update(
    loplayer_t*                player,
    const locommon_input_t*    input,
    const locommon_position_t* cursor
);

void
loplayer_draw(
    const loplayer_t* player
);

void
loplayer_pack(
    const loplayer_t* player,
    msgpack_packer*   packer
);

bool
loplayer_unpack(
    loplayer_t*           player,
    const msgpack_object* obj  /* NULLABLE */
);

void
loplayer_test_packing(
    const loplayer_t* player
);
