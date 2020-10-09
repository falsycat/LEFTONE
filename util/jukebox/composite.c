#include "./composite.h"

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/math/algorithm.h"
#include "util/memory/memory.h"

#include "./effect.h"
#include "./format.h"

struct jukebox_composite_t {
  jukebox_effect_t super;

  jukebox_format_t format;

  atomic_bool playing;

  size_t            effects_reserved;
  size_t            effects_length;
  jukebox_effect_t* effects[1];
};

static void jukebox_composite_affect_(
    jukebox_effect_t* effect, const jukebox_effect_pcm_t* pcm) {
  assert(effect != NULL);
  assert(pcm    != NULL);

# define CHUNK_SIZE 256

  jukebox_composite_t* c = (typeof(c)) effect;
  if (!atomic_load(&c->playing)) return;

  const int32_t channels     = c->format.channels;
  const size_t  chunk_frames = CHUNK_SIZE / channels;

  uint64_t read = 0;
  while (read < (uint64_t) pcm->frames) {
    float chunk[CHUNK_SIZE] = {0};

    jukebox_effect_pcm_t chunk_pcm = {
      .ptr    = chunk,
      .frames = MATH_MIN(chunk_frames, pcm->frames - read),
    };
    for (size_t i = 0; i < c->effects_length; ++i) {
      jukebox_effect_affect(c->effects[i], &chunk_pcm);
    }

    const size_t len = (size_t) chunk_pcm.frames*channels;
    const float* src = chunk;
    float*       dst = pcm->ptr + read * channels;
    for (uint64_t i = 0; i < len; ++i) {
      *(dst++) += *(src++);
    }
    read += chunk_pcm.frames;
  }

# undef CHUNK_SIZE
}

jukebox_composite_t* jukebox_composite_new(
    const jukebox_format_t* format, size_t reserve) {
  assert(jukebox_format_valid(format));
  assert(reserve > 0);

  jukebox_composite_t* compo =
      memory_new(sizeof(*compo) + (reserve-1)*sizeof(compo->effects[0]));
  *compo = (typeof(*compo)) {
    .super = {
      .vtable = {
        .affect = jukebox_composite_affect_,
      },
    },
    .format           = *format,
    .effects_reserved = reserve,
  };
  return compo;
}

void jukebox_composite_delete(jukebox_composite_t* compo) {
  if (compo == NULL) return;

  memory_delete(compo);
}

void jukebox_composite_add_effect(
    jukebox_composite_t* compo, jukebox_effect_t* effect) {
  assert(compo  != NULL);
  assert(effect != NULL);

  assert(!atomic_load(&compo->playing));

  if (compo->effects_length >= compo->effects_reserved) {
    fprintf(stderr, "jukebox: composite effect overflow\n");
    abort();
  }
  compo->effects[compo->effects_length++] = effect;
}

void jukebox_composite_play(jukebox_composite_t* compo) {
  assert(compo != NULL);

  atomic_store(&compo->playing, true);
}

void jukebox_composite_stop(jukebox_composite_t* compo) {
  assert(compo != NULL);

  atomic_store(&compo->playing, false);
}
