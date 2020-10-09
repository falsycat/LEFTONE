#pragma once

#include <stdint.h>

#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loshader/set.h"

typedef struct {
  loshader_set_t*          shaders;
  const locommon_ticker_t* ticker;
  const loplayer_t*        player;

  uint64_t ring_start;
  uint64_t ring_end;
  float    clockhand;
  float    alpha;
} loui_combat_t;

void
loui_combat_initialize(
    loui_combat_t*           combat,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    const loplayer_t*        player
);

void
loui_combat_deinitialize(
    loui_combat_t* combat
);

void
loui_combat_update(
    loui_combat_t* combat
);

void
loui_combat_draw(
    const loui_combat_t* combat
);
