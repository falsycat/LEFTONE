#pragma once

#include "core/locommon/ticker.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./entity.h"
#include "./event.h"
#include "./status.h"

struct loplayer_hud_t;
typedef struct loplayer_hud_t loplayer_hud_t;

loplayer_hud_t*
loplayer_hud_new(
    loresource_set_t*        resource,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    const loplayer_event_t*  event,
    const loplayer_status_t* status,
    const loplayer_entity_t* entity
);

void
loplayer_hud_delete(
    loplayer_hud_t* hud
);

void
loplayer_hud_show(
    loplayer_hud_t* hud
);

void
loplayer_hud_hide(
    loplayer_hud_t* hud
);

void
loplayer_hud_set_biome_text(
    loplayer_hud_t* hud,
    const char*     text
);

void
loplayer_hud_update(
    loplayer_hud_t* hud
);

void
loplayer_hud_draw_ui(
    const loplayer_hud_t* hud
);
