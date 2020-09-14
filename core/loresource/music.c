#include "./music.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/jukebox/amp.h"
#include "util/jukebox/composite.h"
#include "util/jukebox/decoder.h"
#include "util/jukebox/format.h"
#include "util/jukebox/mixer.h"
#include "util/math/rational.h"

/* resources */
#include "anysrc/music/biome_boss.mp3.h"
#include "anysrc/music/biome_cavias_camp.mp3.h"
#include "anysrc/music/biome_laboratory.mp3.h"
#include "anysrc/music/biome_metaphysical_gate.mp3.h"
#include "anysrc/music/boss_big_warder.mp3.h"
#include "anysrc/music/boss_greedy_scientist.mp3.h"
#include "anysrc/music/boss_theists_child.mp3.h"
#include "anysrc/music/boss_theists_child.mp3.h"
#include "anysrc/music/title.mp3.h"

#define LORESOURCE_MUSIC_EACH_(PROC) do {  \
  PROC(biome_boss);  \
  PROC(biome_cavias_camp);  \
  PROC(biome_laboratory);  \
  PROC(biome_metaphysical_gate);  \
  PROC(boss_big_warder);  \
  PROC(boss_greedy_scientist);  \
  PROC(boss_theists_child);  \
  PROC(title);  \
} while (0)

static void loresource_music_player_initialize_(
    loresource_music_player_t* p,
    const jukebox_format_t*    format,
    const void*                buf,
    size_t                     len) {
  assert(p != NULL);
  assert(jukebox_format_valid(format));
  assert(buf != NULL || len == 0);

  *p = (typeof(*p)) {
    .decoder = jukebox_decoder_new_from_memory_mp3(format, buf, len),
    .compo   = jukebox_composite_new(format, 2),
  };
  jukebox_amp_initialize(&p->amp, format);

  jukebox_composite_add_effect(p->compo, (jukebox_effect_t*) p->decoder);
  jukebox_composite_add_effect(p->compo, (jukebox_effect_t*) &p->amp);
  jukebox_composite_play(p->compo);
}

static void loresource_music_player_deinitialize_(
    loresource_music_player_t* p) {
  assert(p != NULL);

  jukebox_composite_delete(p->compo);
  jukebox_amp_deinitialize(&p->amp);
  jukebox_decoder_delete(p->decoder);
}

void loresource_music_initialize(
    loresource_music_t*     music,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format) {
  assert(music != NULL);
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  *music = (typeof(*music)) {0};

# define each_(n) do {  \
    loresource_music_player_initialize_(  \
        &music->n,  \
        format,  \
        loresource_music_##n##_mp3_,  \
        sizeof(loresource_music_##n##_mp3_));  \
    jukebox_mixer_add_effect(mixer, (jukebox_effect_t*) music->n.compo);  \
  } while (0)

  LORESOURCE_MUSIC_EACH_(each_);

# undef each_
}

void loresource_music_deinitialize(loresource_music_t* music) {
  assert(music != NULL);

# define each_(n) do {  \
    loresource_music_player_deinitialize_(&music->n);  \
  } while (0)

  LORESOURCE_MUSIC_EACH_(each_);

# undef each_
}
