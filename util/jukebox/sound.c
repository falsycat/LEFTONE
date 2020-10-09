#include "./sound.h"

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <miniaudio.h>

#include "util/math/rational.h"
#include "util/memory/memory.h"

#include "./effect.h"
#include "./format.h"

struct jukebox_sound_buffer_t {
  jukebox_format_t format;

  uint64_t frames;
  float    ptr[1];
};

struct jukebox_sound_t {
  jukebox_effect_t super;

  const jukebox_sound_buffer_t* buffer;

  atomic_bool stop_all;

  size_t                length;
  atomic_uint_least64_t frames[1];
};

static void jukebox_sound_affect_(
    jukebox_effect_t* effect, const jukebox_effect_pcm_t* pcm) {
  assert(effect != NULL);
  assert(pcm    != NULL);

  jukebox_sound_t* s = (typeof(s)) effect;

  if (atomic_load(&s->stop_all)) {
    for (size_t i = 0; i < s->length; ++i) {
      atomic_store(&s->frames[i], s->buffer->frames);
    }
    atomic_store(&s->stop_all, false);
    return;
  }

  const int32_t ch = s->buffer->format.channels;
  for (size_t i = 0; i < s->length; ++i) {
    const uint64_t frame = atomic_load(&s->frames[i]);
    if (frame >= s->buffer->frames) continue;

    const float* src = s->buffer->ptr;
    float*       dst = pcm->ptr;

    const float* src_ed = src + s->buffer->frames*ch;
    const float* dst_ed = dst + pcm->frames*ch;

    src += frame*ch;
    while (src < src_ed && dst < dst_ed) {
      *(dst++) += *(src++);
    }
    atomic_fetch_add(&s->frames[i], (src - s->buffer->ptr)/ch - frame);
  }
}

static jukebox_sound_buffer_t* jukebox_sound_buffer_new_(
    const jukebox_format_t* format, ma_decoder* decoder) {
  assert(format  != NULL);
  assert(decoder != NULL);

  const uint64_t frames = ma_decoder_get_length_in_pcm_frames(decoder);
  const size_t   length = frames * format->channels;

  jukebox_sound_buffer_t* buf =
      memory_new(sizeof(*buf) + (length-1)*sizeof(buf->ptr[0]));
  *buf = (typeof(*buf)) {
    .format = *format,
    .frames = frames,
  };

  const uint64_t read_frames =
      ma_decoder_read_pcm_frames(decoder, buf->ptr, frames);

  /* usually read_frames should be equal to frames */
  buf->frames = read_frames;
  return buf;
}

jukebox_sound_buffer_t* jukebox_sound_buffer_new_from_file(
    const jukebox_format_t* format, const char* path) {
  assert(jukebox_format_valid(format));

  const ma_decoder_config cfg = ma_decoder_config_init(
      ma_format_f32, format->channels, format->sample_rate);

  ma_decoder decoder;
  if (ma_decoder_init_file(path, &cfg, &decoder) != MA_SUCCESS) {
    fprintf(stderr, "jukebox: invalid audio file '%s'\n", path);
    abort();
  }
  jukebox_sound_buffer_t* sndbuf = jukebox_sound_buffer_new_(format, &decoder);
  ma_decoder_uninit(&decoder);
  return sndbuf;
}

jukebox_sound_buffer_t* jukebox_sound_buffer_new_from_memory_mp3(
    const jukebox_format_t* format, const void* buf, size_t len) {
  assert(jukebox_format_valid(format));
  assert(buf != NULL || len == 0);

  const ma_decoder_config cfg = ma_decoder_config_init(
      ma_format_f32, format->channels, format->sample_rate);

  ma_decoder decoder;
  if (ma_decoder_init_memory_mp3(buf, len, &cfg, &decoder) != MA_SUCCESS) {
    fprintf(stderr, "jukebox: invalid mp3 buffer\n");
    abort();
  }
  jukebox_sound_buffer_t* sndbuf = jukebox_sound_buffer_new_(format, &decoder);
  ma_decoder_uninit(&decoder);
  return sndbuf;
}

void jukebox_sound_buffer_delete(jukebox_sound_buffer_t* buf) {
  if (buf == NULL) return;

  memory_delete(buf);
}

jukebox_sound_t* jukebox_sound_new(
    const jukebox_sound_buffer_t* buf, size_t max_concurrent) {
  assert(buf != NULL);
  assert(max_concurrent > 0);

  jukebox_sound_t* sound =
      memory_new(sizeof(*sound) + (max_concurrent-1)*sizeof(sound->frames[0]));
  *sound = (typeof(*sound)) {
    .super = {
      .vtable = {
        .affect = jukebox_sound_affect_,
      },
    },
    .buffer = buf,
    .length = max_concurrent,
  };

  for (size_t i = 0; i < sound->length; ++i) {
    sound->frames[i] = sound->buffer->frames;
  }
  return sound;
}

void jukebox_sound_delete(jukebox_sound_t* sound) {
  if (sound == NULL) return;

  memory_delete(sound);
}

bool jukebox_sound_play(jukebox_sound_t* sound) {
  assert(sound != NULL);

  if (atomic_load(&sound->stop_all)) return false;

  for (size_t i = 0; i < sound->length; ++i) {
    if (atomic_load(&sound->frames[i]) >= sound->buffer->frames) {
      atomic_store(&sound->frames[i], 0);
      return true;
    }
  }
  return false;
}

void jukebox_sound_stop_all(jukebox_sound_t* sound) {
  assert(sound != NULL);

  atomic_store(&sound->stop_all, true);
}
