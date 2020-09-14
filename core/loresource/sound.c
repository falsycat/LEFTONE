#include "./sound.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "util/jukebox/amp.h"
#include "util/jukebox/composite.h"
#include "util/jukebox/delay.h"
#include "util/jukebox/format.h"
#include "util/jukebox/mixer.h"
#include "util/jukebox/sound.h"
#include "util/math/algorithm.h"
#include "util/math/rational.h"
#include "util/memory/memory.h"

/* resources */
#include "anysrc/sound/bomb.mp3.h"
#include "anysrc/sound/enemy_shoot.mp3.h"
#include "anysrc/sound/enemy_trigger.mp3.h"
#include "anysrc/sound/damage.mp3.h"
#include "anysrc/sound/dodge.mp3.h"
#include "anysrc/sound/dying.mp3.h"
#include "anysrc/sound/guard.mp3.h"
#include "anysrc/sound/player_shoot.mp3.h"
#include "anysrc/sound/player_trigger.mp3.h"
#include "anysrc/sound/reflection.mp3.h"
#include "anysrc/sound/touch_gate.mp3.h"

#define LORESOURCE_SOUND_DECL_(PROC)  \
  PROC(bomb);  \
  PROC(enemy_shoot);  \
  PROC(enemy_trigger);  \
  PROC(damage);  \
  PROC(dodge);  \
  PROC(dying);  \
  PROC(guard);  \
  PROC(player_shoot);  \
  PROC(player_trigger);  \
  PROC(reflection);  \
  PROC(touch_gate);

#define LORESOURCE_SOUND_EACH_(PROC) do {  \
  LORESOURCE_SOUND_DECL_(PROC)  \
} while (0)

struct loresource_sound_t {
  struct {
#   define each_(n)  \
        jukebox_sound_buffer_t* n

    LORESOURCE_SOUND_DECL_(each_);

#   undef each_
  } buffers;

  struct {
#   define each_(n)  \
        jukebox_sound_t* n

    LORESOURCE_SOUND_DECL_(each_);

#   undef each_
  } sounds;

  struct {
    jukebox_delay_t*     delay;
    jukebox_composite_t* compo;
  } combat;
  struct {
    jukebox_delay_t*     delay;
    jukebox_composite_t* compo;
  } env;
  struct {
    jukebox_composite_t* compo;
  } direction;

  struct {
    jukebox_amp_t        amp;
    jukebox_composite_t* compo;
  } root;
};

#define LORESOURCE_SOUND_MAX_CONCURRENT_PLAY 32

static void loresource_sound_initialize_combat_(
    loresource_sound_t*     sound,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format) {
  assert(sound != NULL);
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  typeof(sound->sounds)* s = &sound->sounds;
  typeof(sound->combat)* c = &sound->combat;

  c->delay = jukebox_delay_new(format, &rational(5, 100), .1f, .4f);

  c->compo = jukebox_composite_new(format, 7);
  jukebox_composite_add_effect(c->compo, (jukebox_effect_t*) s->damage);
  jukebox_composite_add_effect(c->compo, (jukebox_effect_t*) s->dodge);
  jukebox_composite_add_effect(c->compo, (jukebox_effect_t*) s->guard);
  jukebox_composite_add_effect(c->compo, (jukebox_effect_t*) s->player_shoot);
  jukebox_composite_add_effect(c->compo, (jukebox_effect_t*) s->player_trigger);
  jukebox_composite_add_effect(c->compo, (jukebox_effect_t*) s->reflection);
  jukebox_composite_add_effect(c->compo, (jukebox_effect_t*) c->delay);

  jukebox_composite_play(c->compo);
}
static void loresource_sound_deinitialize_combat_(loresource_sound_t* sound) {
  assert(sound != NULL);

  typeof(sound->combat)* c = &sound->combat;

  jukebox_composite_delete(c->compo);
  jukebox_delay_delete(c->delay);
}

static void loresource_sound_initialize_env_(
    loresource_sound_t*     sound,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format) {
  assert(sound != NULL);
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  typeof(sound->sounds)* s = &sound->sounds;
  typeof(sound->env)*    e = &sound->env;

  e->delay = jukebox_delay_new(format, &rational(1, 10), .3f, .4f);

  e->compo = jukebox_composite_new(format, 5);
  jukebox_composite_add_effect(e->compo, (jukebox_effect_t*) s->bomb);
  jukebox_composite_add_effect(e->compo, (jukebox_effect_t*) s->enemy_shoot);
  jukebox_composite_add_effect(e->compo, (jukebox_effect_t*) s->enemy_trigger);
  jukebox_composite_add_effect(e->compo, (jukebox_effect_t*) s->touch_gate);
  jukebox_composite_add_effect(e->compo, (jukebox_effect_t*) e->delay);

  jukebox_composite_play(e->compo);
}
static void loresource_sound_deinitialize_env_(loresource_sound_t* sound) {
  assert(sound != NULL);

  typeof(sound->env)* e = &sound->env;

  jukebox_composite_delete(e->compo);
  jukebox_delay_delete(e->delay);
}

static void loresource_sound_initialize_direction_(
    loresource_sound_t*     sound,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format) {
  assert(sound != NULL);
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  typeof(sound->sounds)*    s = &sound->sounds;
  typeof(sound->direction)* d = &sound->direction;

  d->compo = jukebox_composite_new(format, 1);
  jukebox_composite_add_effect(d->compo, (jukebox_effect_t*) s->dying);

  jukebox_composite_play(d->compo);
}
static void loresource_sound_deinitialize_direction_(loresource_sound_t* sound) {
  assert(sound != NULL);

  typeof(sound->direction)* d = &sound->direction;

  jukebox_composite_delete(d->compo);
}

loresource_sound_t* loresource_sound_new(
    jukebox_mixer_t* mixer, const jukebox_format_t* format) {
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  loresource_sound_t* sound = memory_new(sizeof(*sound));
  *sound = (typeof(*sound)) {0};

# define each_(n) do {  \
    sound->buffers.n = jukebox_sound_buffer_new_from_memory_mp3(  \
        format,  \
        loresource_sound_##n##_mp3_,  \
        sizeof(loresource_sound_##n##_mp3_));  \
    sound->sounds.n = jukebox_sound_new(  \
        sound->buffers.n, LORESOURCE_SOUND_MAX_CONCURRENT_PLAY);  \
  } while (0)

    LORESOURCE_SOUND_EACH_(each_);

# undef each_

  sound->root.compo = jukebox_composite_new(format, 4);

  loresource_sound_initialize_combat_(sound, mixer, format);
  jukebox_composite_add_effect(
      sound->root.compo, (jukebox_effect_t*) sound->combat.compo);

  loresource_sound_initialize_env_(sound, mixer, format);
  jukebox_composite_add_effect(
      sound->root.compo, (jukebox_effect_t*) sound->env.compo);

  loresource_sound_initialize_direction_(sound, mixer, format);
  jukebox_composite_add_effect(
      sound->root.compo, (jukebox_effect_t*) sound->direction.compo);

  jukebox_amp_initialize(&sound->root.amp, format);
  jukebox_composite_add_effect(sound->root.compo, &sound->root.amp.super);

  jukebox_amp_change_volume(&sound->root.amp, 1, &rational(1, 1));
  jukebox_composite_play(sound->root.compo);

  jukebox_mixer_add_effect(mixer, (jukebox_effect_t*) sound->root.compo);
  return sound;
}

void loresource_sound_delete(loresource_sound_t* sound) {
  if (sound == NULL) return;

  jukebox_composite_delete(sound->root.compo);
  jukebox_amp_deinitialize(&sound->root.amp);

  loresource_sound_deinitialize_combat_(sound);
  loresource_sound_deinitialize_env_(sound);
  loresource_sound_deinitialize_direction_(sound);

# define each_(n) do {  \
    jukebox_sound_delete(sound->sounds.n);  \
    jukebox_sound_buffer_delete(sound->buffers.n);  \
  } while (0)

    LORESOURCE_SOUND_EACH_(each_);

# undef each_

  memory_delete(sound);
}

void loresource_sound_play(loresource_sound_t* sound, const char* name) {
  assert(sound != NULL);

# define each_(n) do {  \
    if (strcmp(name, #n) == 0) {  \
      jukebox_sound_play(sound->sounds.n);  \
      return;  \
    }  \
  } while (0)

    LORESOURCE_SOUND_EACH_(each_);

# undef each_
}

void loresource_sound_change_master_volume(
    loresource_sound_t* sound, float v, const rational_t* duration) {
  assert(sound != NULL);
  assert(MATH_FLOAT_VALID(v));
  assert(rational_valid(duration));

  jukebox_amp_change_volume(&sound->root.amp, v, duration);
}
