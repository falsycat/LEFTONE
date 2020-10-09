#include "./mixer.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <miniaudio.h>

#include "util/container/array.h"
#include "util/memory/memory.h"

#include "./effect.h"

struct jukebox_mixer_t {
  ma_device device;

  jukebox_format_t format;

  size_t            effects_reserved;
  size_t            effects_length;
  jukebox_effect_t* effects[1];
};

static void jukebox_mixer_device_callback_(
    ma_device* device, void* out, const void* in, ma_uint32 frames) {
  assert(device != NULL);
  assert(out    != NULL);
  assert(frames > 0);

  jukebox_mixer_t* mixer = (typeof(mixer)) device->pUserData;
  assert(mixer != NULL);

  const jukebox_effect_pcm_t pcm = {
    .ptr    = out,
    .frames = frames,
  };
  for (size_t i = 0; i < mixer->effects_length; ++i) {
    jukebox_effect_affect(mixer->effects[i], &pcm);
  }

  (void) in;
}

jukebox_mixer_t* jukebox_mixer_new(
    const jukebox_format_t* format, size_t reserve) {
  assert(jukebox_format_valid(format));
  assert(reserve > 0);

  jukebox_mixer_t* mixer = memory_new(
      sizeof(*mixer) + (reserve-1)*sizeof(mixer->effects[0]));
  *mixer = (typeof(*mixer)) {
    .format           = *format,
    .effects_reserved = reserve,
  };

  ma_device_config config = ma_device_config_init(ma_device_type_playback);
  config.playback.format   = ma_format_f32;
  config.playback.channels = mixer->format.channels;
  config.sampleRate        = mixer->format.sample_rate;
  config.dataCallback      = jukebox_mixer_device_callback_;
  config.pUserData         = mixer;

  if (ma_device_init(NULL, &config, &mixer->device) != MA_SUCCESS) {
    fprintf(stderr, "failed to open audio device\n");
    abort();
  }

  if (ma_device_start(&mixer->device) != MA_SUCCESS) {
    fprintf(stderr, "failed to start audio device\n");
    ma_device_uninit(&mixer->device);
    abort();
  }

  return mixer;
}

void jukebox_mixer_delete(jukebox_mixer_t* mixer) {
  if (mixer == NULL) return;

  ma_device_uninit(&mixer->device);

  memory_delete(mixer);
}

void jukebox_mixer_add_effect(
    jukebox_mixer_t* mixer, jukebox_effect_t* effect) {
  assert(mixer  != NULL);
  assert(effect != NULL);

  ma_mutex_lock(&mixer->device.lock);
  if (mixer->effects_length >= mixer->effects_reserved) {
    fprintf(stderr, "mixer effects overflow\n");
    abort();
  }
  mixer->effects[mixer->effects_length++] = effect;
  ma_mutex_unlock(&mixer->device.lock);
}
