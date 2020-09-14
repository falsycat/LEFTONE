#include "./lowpass.h"

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>

#include "util/math/algorithm.h"

#include "./effect.h"
#include "./format.h"

static void jukebox_lowpass_affect_(
    jukebox_effect_t* effect, const jukebox_effect_pcm_t* pcm) {
  assert(effect != NULL);
  assert(pcm    != NULL);

  jukebox_lowpass_t* lp = (typeof(lp)) effect;

  const size_t ch = lp->format.channels;
  const float  f  = atomic_load(&lp->factor) * JUKEBOX_LOWPASS_FACTOR_UNIT;

  assert(ch <= JUKEBOX_LOWPASS_SUPPORT_MAX_CHANNELS);

  const size_t pcmlen = pcm->frames * ch;
  for (size_t i = 0; i < pcmlen; ++i) {
    const float p = (ch+1 <= i? pcm->ptr[i-1-ch]: lp->prev[i]);
    pcm->ptr[i] = (1.0f-f)*pcm->ptr[i] + f*p;
  }

  if (pcmlen < ch) return;
  for (size_t i = 0; i < ch; ++i) {
    lp->prev[i] = pcm->ptr[pcmlen-ch+i];
  }
}

void jukebox_lowpass_initialize(
    jukebox_lowpass_t* lowpass, const jukebox_format_t* format, float factor) {
  assert(lowpass != NULL);
  assert(jukebox_format_valid(format));
  assert(MATH_FLOAT_VALID(factor));

  *lowpass = (typeof(*lowpass)) {
    .super = {
      .vtable = {
        .affect = jukebox_lowpass_affect_,
      },
    },
    .format = *format,
    .factor = factor*10000,
    .prev   = {0},
  };
}

void jukebox_lowpass_deinitialize(jukebox_lowpass_t* lowpass) {
  assert(lowpass != NULL);

}
