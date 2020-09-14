#include "./beep.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stddef.h>

#include "util/math/algorithm.h"
#include "util/math/constant.h"

#include "./effect.h"
#include "./format.h"

static void jukebox_beep_affect_(
    jukebox_effect_t* effect, const jukebox_effect_pcm_t* pcm) {
  assert(effect != NULL);
  assert(pcm    != NULL);

  jukebox_beep_t* b = (typeof(b)) effect;
  if (!atomic_load(&b->playing)) return;

  const int32_t sample_rate = b->format.sample_rate;
  const int32_t channels    = b->format.channels;
  const float   omega       = 2 * MATH_PI * b->hz;

  float* ptr = pcm->ptr;
  for (int32_t i = 0; i < pcm->frames; ++i) {
    const float a = fmod(i*omega/sample_rate, 2*MATH_PI);
    const float x = sin(a + b->phase) * b->amp;
    for (int32_t j = 0; j < channels; ++j) {
      *(ptr++) += x;
    }
  }

  b->phase += pcm->frames*1.0f/sample_rate * omega;
  b->phase  = fmod(b->phase, 2*MATH_PI);
}

void jukebox_beep_initialize(jukebox_beep_t* beep, const jukebox_format_t* format) {
  assert(beep != NULL);
  assert(jukebox_format_valid(format));

  *beep = (typeof(*beep)) {
    .super = {
      .vtable = {
        .affect = jukebox_beep_affect_,
      },
    },
    .format = *format,
  };
}

void jukebox_beep_deinitialize(jukebox_beep_t* beep) {
  assert(beep != NULL);

}

void jukebox_beep_play(jukebox_beep_t* beep, float hz, float amp) {
  assert(beep != NULL);
  assert(MATH_FLOAT_VALID(hz)  && hz > 0);
  assert(MATH_FLOAT_VALID(amp) && amp > 0);

  if (beep->playing) return;

  beep->hz    = hz;
  beep->amp   = amp;
  beep->phase = 0;
  atomic_store(&beep->playing, true);
}

void jukebox_beep_stop(jukebox_beep_t* beep) {
  assert(beep != NULL);

  if (!beep->playing) return;
  atomic_store(&beep->playing, false);
}
