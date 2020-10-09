#pragma once

#include "util/jukebox/format.h"
#include "util/jukebox/mixer.h"

#include "./font.h"
#include "./language.h"
#include "./music.h"
#include "./sound.h"

typedef struct {
  loresource_sound_set_t sound;
  loresource_music_set_t music;
  loresource_font_t      font;

  loresource_language_t lang;
} loresource_set_t;

void
loresource_set_initialize(
    loresource_set_t*       res,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format,
    loresource_language_t   lang
);

void
loresource_set_deinitialize(
    loresource_set_t* res
);

void
loresource_set_change_language(
    loresource_set_t*     res,
    loresource_language_t lang
);
