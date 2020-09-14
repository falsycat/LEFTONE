#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/lobullet/pool.h"
#include "core/locommon/ticker.h"
#include "core/loresource/set.h"

#include "./camera.h"
#include "./combat.h"
#include "./controller.h"
#include "./entity.h"
#include "./event.h"
#include "./hud.h"
#include "./menu.h"
#include "./status.h"

struct loplayer_action_t;
typedef struct loplayer_action_t loplayer_action_t;

loplayer_action_t*  /* OWNERSHIP */
loplayer_action_new(
    loresource_set_t*            res,
    const locommon_ticker_t*     ticker,
    lobullet_pool_t*             bullets,
    loentity_store_t*            entities,
    loplayer_event_t*            event,
    loplayer_status_t*           status,
    loplayer_entity_t*           entity,
    loplayer_combat_t*           combat,
    const loplayer_controller_t* controller,
    loplayer_camera_t*           camera,
    loplayer_hud_t*              hud,
    loplayer_menu_t*             menu
);

void
loplayer_action_delete(
    loplayer_action_t* action  /* OWNERSHIP */
);

void
loplayer_action_start_menu_popup_state(
    loplayer_action_t* action
);

void
loplayer_action_execute(
    loplayer_action_t* action
);

void
loplayer_action_pack(
    const loplayer_action_t* action,
    msgpack_packer*          packer
);

bool
loplayer_action_unpack(
    loplayer_action_t*    action,
    const msgpack_object* obj  /* NULLABLE */
);
