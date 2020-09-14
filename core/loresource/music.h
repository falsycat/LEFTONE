#pragma once

#include "util/jukebox/amp.h"
#include "util/jukebox/composite.h"
#include "util/jukebox/decoder.h"
#include "util/jukebox/mixer.h"
#include "util/math/rational.h"

typedef struct {
  jukebox_decoder_t*   decoder;
  jukebox_amp_t        amp;
  jukebox_composite_t* compo;
} loresource_music_player_t;

struct loresource_music_t;
typedef struct {
  loresource_music_player_t
      biome_boss,
      biome_cavias_camp,
      biome_metaphysical_gate,
      biome_laboratory,
      boss_big_warder,
      boss_greedy_scientist,
      boss_theists_child,
      title;
} loresource_music_t;

void
loresource_music_initialize(
    loresource_music_t*     music,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format
);

void
loresource_music_deinitialize(
    loresource_music_t* music
);
