#include "./delay.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/algorithm.h"
#include "util/math/rational.h"
#include "util/memory/memory.h"

#include "./effect.h"
#include "./format.h"

struct jukebox_delay_t {
  jukebox_effect_t super;
  jukebox_format_t format;

  float source;
  float feedback;

  size_t length;
  size_t offset;
  size_t cursor;
  float  buffer[1];
};

static void jukebox_delay_affect_(
    jukebox_effect_t* effect, const jukebox_effect_pcm_t* pcm) {
  assert(effect != NULL);
  assert(pcm    != NULL);

  jukebox_delay_t* d = (typeof(d)) effect;

  const int32_t ch     = d->format.channels;
  const size_t  pcmlen = pcm->frames*ch;

  for (size_t i = 0; i < pcmlen; ++i) {
    float* bufoff = &d->buffer[d->offset];
    float* bufcur = &d->buffer[d->cursor];
    float* pcmptr = &pcm->ptr[i];

    *bufoff *= d->feedback;
    *bufoff += *pcmptr * d->source;

    *pcmptr += *bufcur;
    if (MATH_ABS(*pcmptr) >= 1.0f) {
      *pcmptr = MATH_SIGN(*pcmptr) * 0.99f;
    }

    if (++d->offset >= d->length) d->offset = 0;
    if (++d->cursor >= d->length) d->cursor = 0;
  }
}

jukebox_delay_t* jukebox_delay_new(
    const jukebox_format_t* format,
    const rational_t*       duration,
    float                   source_attenuation,
    float                   feedback_attenuation) {
  assert(jukebox_format_valid(format));
  assert(rational_valid(duration));
  assert(MATH_FLOAT_VALID(source_attenuation));
  assert(MATH_FLOAT_VALID(feedback_attenuation));

  rational_t dur_r = rational(format->channels*duration->num, duration->den);
  rational_normalize(&dur_r, format->sample_rate);

  const size_t length = dur_r.num;

  jukebox_delay_t* d =
      memory_new(sizeof(*d) + (length-1)*sizeof(d->buffer[0]));
  *d = (typeof(*d)) {
    .super = {
      .vtable = {
        .affect = jukebox_delay_affect_,
      },
    },
    .format   = *format,
    .source   = source_attenuation,
    .feedback = feedback_attenuation,
    .length   = length,
    .offset   = dur_r.num,
    .cursor   = 0,
  };

  for (size_t i = 0; i < d->length; ++i) d->buffer[i] = 0;
  return d;
}

void jukebox_delay_delete(jukebox_delay_t* delay) {
  if (delay == NULL) return;

  memory_delete(delay);
}
