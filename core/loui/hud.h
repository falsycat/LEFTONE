#pragma once

#include <stdbool.h>

#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"
#include "util/math/vector.h"

#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"
#include "core/loworld/chunk.h"

typedef struct {
  glyphas_block_t* text;

  float alpha;
  float value;
} loui_hud_effect_t;

typedef struct {
  loui_hud_effect_t amnesia;
  loui_hud_effect_t curse;
} loui_hud_effect_set_t;

#define LOUI_HUD_EFFECT_COUNT  \
    (sizeof(loui_hud_effect_set_t)/sizeof(loui_hud_effect_t))

typedef struct {
  loshader_set_t*          shaders;
  const locommon_ticker_t* ticker;
  loplayer_t*              player;

  struct {
    glyphas_cache_t* effect;
    glyphas_cache_t* biome;
  } font;

  struct {
    vec2_t padding;

    vec2_t madness_pos;
    vec2_t madness_sz;
    vec2_t faith_pos;
    vec2_t faith_sz;

    vec2_t effect_fontpx;
    vec2_t effect_fontsz;
    vec2_t effect_pos;
    vec2_t effect_sz;

    vec2_t biome_fontpx;
    vec2_t biome_fontsz;
    vec2_t biome_pos;
  } geo;

  union {
    loui_hud_effect_set_t set;
    loui_hud_effect_t     arr[LOUI_HUD_EFFECT_COUNT];
  } effects;

  glyphas_block_t* biomes[LOWORLD_CHUNK_BIOME_COUNT];
  loworld_chunk_biome_t current_biome;

  float prev_madness;
  float prev_faith;

  float alpha;
} loui_hud_t;

void
loui_hud_initialize(
    loui_hud_t*              hud,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loplayer_t*              player
);

void
loui_hud_deinitialize(
    loui_hud_t* hud
);

void
loui_hud_update(
    loui_hud_t* hud
);

void
loui_hud_draw(
    const loui_hud_t* hud
);
