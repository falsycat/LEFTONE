#include "./effect.h"

#include <assert.h>
#include <stddef.h>

void jukebox_effect_affect(
    jukebox_effect_t* snd, const jukebox_effect_pcm_t* pcm) {
  assert(snd != NULL);
  assert(pcm != NULL);

  assert(snd->vtable.affect != NULL);
  snd->vtable.affect(snd, pcm);
}
