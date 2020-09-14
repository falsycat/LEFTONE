#include "./amp.h"

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/algorithm.h"
#include "util/math/rational.h"

#include "./effect.h"
#include "./format.h"

/* FIXME: Whether this solution works fine,
          depends on how long the operation is done. */

static void jukebox_amp_affect_(
    jukebox_effect_t* effect, const jukebox_effect_pcm_t* pcm) {
  assert(effect != NULL);
  assert(pcm    != NULL);

  jukebox_amp_t* amp = (typeof(amp)) effect;

  const uint64_t easedur = atomic_load(&amp->ease_duration);
  const uint64_t elapsed = atomic_load(&amp->elapsed);

  const float prev = atomic_load(&amp->prev_amount)/1000.0f;
  const float next = atomic_load(&amp->next_amount)/1000.0f;

  float* ptr = pcm->ptr;
  for (int32_t i = 0; i < pcm->frames; ++i) {
    float a = next;
    if (elapsed < easedur) {
      a = (prev-next) * (easedur-elapsed) / easedur + next;
    }

    for (int32_t j = 0; j < amp->format.channels; ++j) {
      *(ptr++) *= a;
    }
  }
  atomic_fetch_add(&amp->elapsed, pcm->frames);
}

void jukebox_amp_initialize(
    jukebox_amp_t* amp, const jukebox_format_t* format) {
  assert(amp    != NULL);
  assert(jukebox_format_valid(format));

  *amp = (typeof(*amp)) {
    .super = {
      .vtable = {
        .affect = jukebox_amp_affect_,
      },
    },
    .format = *format,
  };
}

void jukebox_amp_deinitialize(jukebox_amp_t* amp) {
  assert(amp != NULL);

}

void jukebox_amp_change_volume(
    jukebox_amp_t* amp, float amount, const rational_t* duration) {
  assert(amp != NULL);
  assert(MATH_FLOAT_VALID(amount));
  assert(amount >= 0);

  const float srate = amp->format.sample_rate;

  atomic_store(&amp->ease_duration, 0);

  atomic_store(&amp->elapsed, 0);
  atomic_store(&amp->prev_amount, amp->next_amount);

  rational_t dur = rational(1, 100);
  if (rational_valid(duration)) rational_addeq(&dur, duration);
  rational_normalize(&dur, srate);

  atomic_store(&amp->next_amount, (uint16_t) (amount*1000));
  atomic_store(&amp->ease_duration, dur.num);
}
