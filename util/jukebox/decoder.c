#include "./decoder.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "thirdparty/miniaudio/miniaudio.h"

#include "util/math/algorithm.h"
#include "util/math/rational.h"
#include "util/memory/memory.h"

#include "./effect.h"
#include "./format.h"

# define JUKEBOX_DECODER_AFFECTION_CHUNK_SIZE 256

struct jukebox_decoder_t {
  jukebox_effect_t super;

  jukebox_format_t format;

  ma_decoder ma;

  atomic_bool          playing;
  atomic_int_least64_t duration;  /* negative = 0 */

  atomic_uint_least64_t current;  /* main thread doesn't modify when playing*/
  bool                  loop;
  bool                  request_seek;
};

static void jukebox_decoder_seek_(jukebox_decoder_t* dec, uint64_t f) {
  assert(dec != NULL);
  assert(atomic_load(&dec->playing));

  if (ma_decoder_seek_to_pcm_frame(&dec->ma, f) != MA_SUCCESS) {
    fprintf(stderr, "jukebox: failed to seek ma_decoder\n");
    abort();
  }
  atomic_store(&dec->current, f);
  dec->request_seek = false;
}

static void jukebox_decoder_affect_(
    jukebox_effect_t* effect, const jukebox_effect_pcm_t* pcm) {
  assert(effect != NULL);
  assert(pcm    != NULL);

  jukebox_decoder_t* dec = (typeof(dec)) effect;
  if (!atomic_load(&dec->playing)) return;

  if (dec->request_seek) {
    jukebox_decoder_seek_(dec, atomic_load(&dec->current));
  }

  const int32_t ch = dec->format.channels;

  uint64_t read_frames = 0;
  for (;;) {
    const int64_t  duration         = atomic_load(&dec->duration);
    const uint64_t requested_frames = pcm->frames - read_frames;

    if (duration <= 0) {
      atomic_store(&dec->playing, false);
      break;
    }
    if (read_frames >= (uint64_t) pcm->frames) break;

    uint64_t chunk_frames = JUKEBOX_DECODER_AFFECTION_CHUNK_SIZE / ch;
    if (chunk_frames > (uint64_t) duration) chunk_frames = duration;
    if (chunk_frames > requested_frames)    chunk_frames = requested_frames;

    float chunk[JUKEBOX_DECODER_AFFECTION_CHUNK_SIZE];

    const uint64_t decoded_frames =
        ma_decoder_read_pcm_frames(&dec->ma, chunk, chunk_frames);
    if (decoded_frames != chunk_frames) {
      jukebox_decoder_seek_(dec, 0);
    }

    assert(read_frames+decoded_frames <= (uint64_t) pcm->frames);
    for (size_t i = 0; i < decoded_frames*ch; ++i) {
      pcm->ptr[read_frames*ch+i] += chunk[i];
    }
    read_frames += decoded_frames;

    atomic_fetch_add(&dec->duration, -decoded_frames);
    atomic_fetch_add(&dec->current,   decoded_frames);
  }
}

static jukebox_decoder_t* jukebox_decoder_new_(
    const jukebox_format_t* format) {
  assert(jukebox_format_valid(format));

  jukebox_decoder_t* decoder = memory_new(sizeof(*decoder));
  *decoder = (typeof(*decoder)) {
    .super = {
      .vtable = {
        .affect = jukebox_decoder_affect_,
      },
    },
    .format = *format,
  };
  return decoder;
}

jukebox_decoder_t* jukebox_decoder_new_from_file(
    const jukebox_format_t* format, const char* path) {
  assert(jukebox_format_valid(format));

  jukebox_decoder_t* decoder = jukebox_decoder_new_(format);

  const ma_decoder_config cfg = ma_decoder_config_init(
      ma_format_f32, decoder->format.channels, decoder->format.sample_rate);

  if (ma_decoder_init_file(path, &cfg, &decoder->ma) != MA_SUCCESS) {
    fprintf(stderr, "jukebox: invalid audio file '%s'\n", path);
    abort();
  }
  return decoder;
}

jukebox_decoder_t* jukebox_decoder_new_from_memory_mp3(
    const jukebox_format_t* format, const void* buf, size_t len) {
  assert(jukebox_format_valid(format));
  assert(buf != NULL || len == 0);

  jukebox_decoder_t* decoder = jukebox_decoder_new_(format);

  const ma_decoder_config cfg = ma_decoder_config_init(
      ma_format_f32, decoder->format.channels, decoder->format.sample_rate);

  if (ma_decoder_init_memory_mp3(buf, len, &cfg, &decoder->ma) != MA_SUCCESS) {
    fprintf(stderr, "jukebox: invalid mp3 buffer\n");
    abort();
  }
  return decoder;
}

void jukebox_decoder_delete(jukebox_decoder_t* decoder) {
  if (decoder == NULL) return;

  ma_decoder_uninit(&decoder->ma);

  memory_delete(decoder);
}

void jukebox_decoder_play(
    jukebox_decoder_t* decoder, const rational_t* st, bool loop) {
  assert(decoder != NULL);
  assert(rational_valid(st));

  if (atomic_load(&decoder->playing)) return;

  rational_t t = *st;
  rational_normalize(&t, decoder->format.sample_rate);

  atomic_store(&decoder->duration, INT64_MAX);

  decoder->loop         = loop;
  decoder->request_seek = true;

  atomic_store(&decoder->current, t.num);
  atomic_store(&decoder->playing, true);
}

void jukebox_decoder_resume(jukebox_decoder_t* decoder, bool loop) {
  assert(decoder != NULL);

  if (atomic_load(&decoder->playing)) return;

  atomic_store(&decoder->duration, INT64_MAX);

  decoder->loop = loop;

  atomic_store(&decoder->playing, true);
}

void jukebox_decoder_stop_after(
    jukebox_decoder_t* decoder, const rational_t* dur) {
  assert(decoder != NULL);
  assert(rational_valid(dur));

  if (!atomic_load(&decoder->playing)) return;

  rational_t d = *dur;
  rational_normalize(&d, decoder->format.sample_rate);

  atomic_store(&decoder->duration, d.num);
}

void jukebox_decoder_get_seek_position(
    const jukebox_decoder_t* decoder, rational_t* time) {
  assert(decoder != NULL);
  assert(time    != NULL);

  *time = (typeof(*time)) {
    .num = atomic_load(&decoder->current),
    .den = decoder->format.sample_rate,
  };
}

void jukebox_decoder_get_duration(
    const jukebox_decoder_t* decoder, rational_t* time) {
  assert(decoder != NULL);
  assert(time    != NULL);

  *time = rational(
      ma_decoder_get_length_in_pcm_frames((ma_decoder*) &decoder->ma),
      decoder->format.sample_rate);
}
