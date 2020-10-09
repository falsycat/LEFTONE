#pragma once

#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"
#include "util/math/vector.h"

#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loplayer/popup.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

typedef struct {
  loshader_set_t*          shaders;
  const locommon_ticker_t* ticker;
  loplayer_t*              player;

  struct {
    struct {
      glyphas_cache_t* big;
    } serif;
  } font;

  struct {
    vec2_t big_fontpx;
    vec2_t big_fontsz;
  } geo;

  float                 alpha;
  loplayer_popup_item_t item;

  struct {
    glyphas_block_t* new_stance;
  } head;
} loui_popup_t;

void
loui_popup_initialize(
    loui_popup_t*            popup,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loplayer_t*              player
);

void
loui_popup_deinitialize(
    loui_popup_t* popup
);

void
loui_popup_update(
    loui_popup_t* popup
);

void
loui_popup_draw(
    const loui_popup_t* popup
);
