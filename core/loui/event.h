#pragma once

#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"

#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

typedef struct {
  loresource_set_t*        res;
  loshader_set_t*          shaders;
  const locommon_screen_t* screen;
  const locommon_ticker_t* ticker;
  loplayer_t*              player;

  glyphas_cache_t* font;

  float cinescope;

  glyphas_block_t* line;
  uint64_t         last_line_update;
} loui_event_t;

void
loui_event_initialize(
    loui_event_t*            ev,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loplayer_t*              player
);

void
loui_event_deinitialize(
    loui_event_t* ev
);

void
loui_event_update(
    loui_event_t* ev
);

void
loui_event_draw(
    const loui_event_t* ev
);
