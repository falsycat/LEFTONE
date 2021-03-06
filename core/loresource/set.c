#include "./set.h"

#include <assert.h>
#include <stddef.h>

#include "util/jukebox/format.h"
#include "util/jukebox/mixer.h"

#include "./font.h"
#include "./language.h"
#include "./music.h"
#include "./sound.h"
#include "./text.h"

void loresource_set_initialize(
    loresource_set_t*       res,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format,
    loresource_language_t   lang) {
  assert(res   != NULL);
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  *res = (typeof(*res)) {0};

  loresource_sound_set_initialize(&res->sound, mixer, format);
  loresource_music_set_initialize(&res->music, mixer, format);
  loresource_font_initialize(&res->font);
  loresource_set_change_language(res, lang);
}

void loresource_set_deinitialize(loresource_set_t* res) {
  assert(res != NULL);

  loresource_font_deinitialize(&res->font);
  loresource_music_set_deinitialize(&res->music);
  loresource_sound_set_deinitialize(&res->sound);
}

void loresource_set_change_language(
    loresource_set_t* res, loresource_language_t lang) {
  assert(res != NULL);

  res->lang = lang;
  loresource_text_optimize(res->lang);
}
