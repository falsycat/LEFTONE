#pragma once

#include <stdint.h>

#include "./format.h"

struct jukebox_effect_t;
typedef struct jukebox_effect_t jukebox_effect_t;

typedef struct {
  float*  ptr;
  int32_t frames;
} jukebox_effect_pcm_t;

typedef struct {
  void
  (*affect)(
      jukebox_effect_t*           snd,
      const jukebox_effect_pcm_t* pcm
  );
} jukebox_effect_vtable_t;

struct jukebox_effect_t {
  jukebox_effect_vtable_t vtable;
};

void
jukebox_effect_affect(
    jukebox_effect_t*           snd,
    const jukebox_effect_pcm_t* pcm
);
