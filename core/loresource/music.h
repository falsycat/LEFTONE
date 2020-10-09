#pragma once

#include "util/jukebox/amp.h"
#include "util/jukebox/composite.h"
#include "util/jukebox/decoder.h"
#include "util/jukebox/mixer.h"

typedef enum {
  /* BENUM BEGIN loresource_music_id */
  LORESOURCE_MUSIC_ID_BIOME_BOSS,
  LORESOURCE_MUSIC_ID_BIOME_CAVIAS_CAMP,
  LORESOURCE_MUSIC_ID_BIOME_METAPHYSICAL_GATE,
  LORESOURCE_MUSIC_ID_BIOME_LABORATORY,
  LORESOURCE_MUSIC_ID_BOSS_BIG_WARDER,
  LORESOURCE_MUSIC_ID_BOSS_GREEDY_SCIENTIST,
  LORESOURCE_MUSIC_ID_BOSS_THEISTS_CHILD,
  LORESOURCE_MUSIC_ID_TITLE,
  /* BENUM END */
} loresource_music_id_t;

#include "core/loresource/benum/music.h"

typedef struct {
  loresource_music_id_t id;
  const char*           name;

  jukebox_decoder_t*   decoder;
  jukebox_amp_t        amp;
  jukebox_composite_t* compo;
} loresource_music_t;

typedef struct {
  loresource_music_t items[LORESOURCE_MUSIC_ID_COUNT];
} loresource_music_set_t;

void
loresource_music_set_initialize(
    loresource_music_set_t* set,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format
);

void
loresource_music_set_deinitialize(
    loresource_music_set_t* set
);

loresource_music_t*
loresource_music_set_get(
    loresource_music_set_t* set,
    loresource_music_id_t   id
);
