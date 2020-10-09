#include "./music.h"

#include <assert.h>
#include <stddef.h>

#include "util/jukebox/amp.h"
#include "util/jukebox/composite.h"
#include "util/jukebox/decoder.h"
#include "util/jukebox/format.h"
#include "util/jukebox/mixer.h"

/* resources */
#include "core/loresource/anysrc/music/biome_boss.mp3.h"
#include "core/loresource/anysrc/music/biome_cavias_camp.mp3.h"
#include "core/loresource/anysrc/music/biome_laboratory.mp3.h"
#include "core/loresource/anysrc/music/biome_metaphysical_gate.mp3.h"
#include "core/loresource/anysrc/music/boss_big_warder.mp3.h"
#include "core/loresource/anysrc/music/boss_greedy_scientist.mp3.h"
#include "core/loresource/anysrc/music/boss_theists_child.mp3.h"
#include "core/loresource/anysrc/music/boss_theists_child.mp3.h"
#include "core/loresource/anysrc/music/title.mp3.h"

static void loresource_music_initialize_(
    loresource_music_t*     m,
    const jukebox_format_t* format,
    const void*             buf,
    size_t                  len) {
  assert(m != NULL);
  assert(jukebox_format_valid(format));
  assert(buf != NULL || len == 0);

  *m = (typeof(*m)) {
    .decoder = jukebox_decoder_new_from_memory_mp3(format, buf, len),
    .compo   = jukebox_composite_new(format, 2),
  };
  jukebox_amp_initialize(&m->amp, format);

  jukebox_composite_add_effect(m->compo, (jukebox_effect_t*) m->decoder);
  jukebox_composite_add_effect(m->compo, (jukebox_effect_t*) &m->amp);
  jukebox_composite_play(m->compo);
}
static void loresource_music_deinitialize_(loresource_music_t* m) {
  assert(m != NULL);

  jukebox_composite_delete(m->compo);
  jukebox_amp_deinitialize(&m->amp);
  jukebox_decoder_delete(m->decoder);
}

void loresource_music_set_initialize(
    loresource_music_set_t* set,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format) {
  assert(set   != NULL);
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  *set = (typeof(*set)) {0};

# define each_(N, n) do {  \
    loresource_music_t* p =  \
        &set->items[LORESOURCE_MUSIC_ID_##N];  \
    *p = (typeof(*p)) {  \
      .id   = LORESOURCE_MUSIC_ID_##N,  \
      .name = #N,  \
    };  \
    loresource_music_initialize_(  \
        p,  \
        format,  \
        loresource_music_##n##_mp3_,  \
        sizeof(loresource_music_##n##_mp3_));  \
    jukebox_mixer_add_effect(mixer, (jukebox_effect_t*) p->compo);  \
  } while (0)
  LORESOURCE_MUSIC_ID_EACH(each_);
# undef each_
}

void loresource_music_set_deinitialize(loresource_music_set_t* set) {
  assert(set != NULL);

  for (size_t i = 0; i < LORESOURCE_MUSIC_ID_COUNT; ++i) {
    loresource_music_deinitialize_(&set->items[i]);
  }
}

loresource_music_t* loresource_music_set_get(
    loresource_music_set_t* set, loresource_music_id_t id) {
  assert(set != NULL);

  return &set->items[id];
}
