#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loresource/music.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./view.h"

typedef struct {
  bool disable_heavy_backwall;
  bool disable_heavy_fog;
} loworld_environment_config_t;

typedef struct {
  /* injected deps */
  loresource_set_t*        res;
  loshader_set_t*          shaders;
  const locommon_ticker_t* ticker;
  const loworld_view_t*    view;
  loplayer_t*              player;

  /* immutable params */
  loworld_environment_config_t config;

  /* read-only mutable params */
  float                 transition;
  loworld_chunk_biome_t biome;

  loresource_music_player_t* music;
  bool                       music_control;
  bool                       sound_attenuation;

  loshader_backwall_drawer_param_t backwall;
  loshader_fog_drawer_param_t      fog;
} loworld_environment_t;

void
loworld_environment_initialize(
    loworld_environment_t*              env,
    loresource_set_t*                   res,
    loshader_set_t*                     shaders,
    const locommon_ticker_t*            ticker,
    const loworld_view_t*               view,
    loplayer_t*                         player,
    const loworld_environment_config_t* config
);

void
loworld_environment_deinitialize(
    loworld_environment_t* env
);

void
loworld_environment_update(
    loworld_environment_t* env
);

void
loworld_environment_draw(
    const loworld_environment_t* env
);
