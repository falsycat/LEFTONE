#pragma once

#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"
#include "util/math/vector.h"

#include "core/locommon/input.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loplayer/stance.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

typedef struct {
  glyphas_block_t* name;
  glyphas_block_t* desc;
  glyphas_block_t* note;

  vec2_t pos;
  float  highlight;
} loui_menu_stance_t;

typedef struct {
  loshader_set_t*          shaders;
  const locommon_ticker_t* ticker;
  loplayer_t*              player;

  struct {
    struct {
      glyphas_cache_t* big;
      glyphas_cache_t* normal;
    } sans;
    struct {
      glyphas_cache_t* small;
    } serif;
  } font;

  struct {
    vec2_t big_fontpx;
    vec2_t big_fontsz;

    vec2_t normal_fontpx;
    vec2_t normal_fontsz;

    vec2_t small_fontpx;
    vec2_t small_fontsz;

    vec2_t stance_icon_interval;
    vec2_t stance_iconsz;
  } geo;

  loui_menu_stance_t  stances[LOPLAYER_STANCE_COUNT+1];
  loui_menu_stance_t* hovered_stance;

  glyphas_block_t* exit_text;
  bool             request_exit;

  float alpha;
} loui_menu_t;

void
loui_menu_initialize(
    loui_menu_t*             menu,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loplayer_t*              player
);

void
loui_menu_deinitialize(
    loui_menu_t* menu
);

void
loui_menu_update(
    loui_menu_t*            menu,
    const locommon_input_t* input  /* NULLABLE */
);

void
loui_menu_draw(
    const loui_menu_t* menu
);
