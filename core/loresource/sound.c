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

/* resources */
#include "core/loresource/anysrc/sound/bomb.mp3.h"
#include "core/loresource/anysrc/sound/enemy_shoot.mp3.h"
#include "core/loresource/anysrc/sound/enemy_trigger.mp3.h"
#include "core/loresource/anysrc/sound/damage.mp3.h"
#include "core/loresource/anysrc/sound/dodge.mp3.h"
#include "core/loresource/anysrc/sound/dying.mp3.h"
#include "core/loresource/anysrc/sound/guard.mp3.h"
#include "core/loresource/anysrc/sound/player_shoot.mp3.h"
#include "core/loresource/anysrc/sound/player_trigger.mp3.h"
#include "core/loresource/anysrc/sound/reflection.mp3.h"
#include "core/loresource/anysrc/sound/touch_gate.mp3.h"

#define MAX_CONCURRENT_PLAYS_ 32

static void loresource_sound_set_initialize_combat_(
    loresource_sound_set_t* set,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format) {
  assert(set   != NULL);
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  typeof(set->combat)* c = &set->combat;

  c->delay = jukebox_delay_new(format, &rational(5, 100), .1f, .4f);

  c->compo = jukebox_composite_new(format, 7);
# define add_(NAME)  \
      jukebox_composite_add_effect(  \
        c->compo,  \
        (jukebox_effect_t*) set->items[LORESOURCE_SOUND_ID_##NAME].sound)
  add_(DAMAGE);
  add_(DODGE);
  add_(GUARD);
  add_(PLAYER_SHOOT);
  add_(PLAYER_TRIGGER);
  add_(REFLECTION);
# undef add_

  jukebox_composite_add_effect(c->compo, (jukebox_effect_t*) c->delay);
  jukebox_composite_play(c->compo);
}
static void loresource_sound_set_deinitialize_combat_(
    loresource_sound_set_t* set) {
  assert(set != NULL);

  typeof(set->combat)* c = &set->combat;

  jukebox_composite_delete(c->compo);
  jukebox_delay_delete(c->delay);
}

static void loresource_sound_set_initialize_env_(
    loresource_sound_set_t* set,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format) {
  assert(set   != NULL);
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  typeof(set->env)* e = &set->env;

  e->delay = jukebox_delay_new(format, &rational(1, 10), .3f, .4f);

  e->compo = jukebox_composite_new(format, 5);
# define add_(NAME)  \
      jukebox_composite_add_effect(  \
        e->compo,  \
        (jukebox_effect_t*) set->items[LORESOURCE_SOUND_ID_##NAME].sound)
  add_(BOMB);
  add_(ENEMY_SHOOT);
  add_(ENEMY_TRIGGER);
  add_(TOUCH_GATE);
# undef add_

  jukebox_composite_add_effect(e->compo, (jukebox_effect_t*) e->delay);
  jukebox_composite_play(e->compo);
}
static void loresource_sound_set_deinitialize_env_(
    loresource_sound_set_t* set) {
  assert(set != NULL);

  typeof(set->env)* e = &set->env;

  jukebox_composite_delete(e->compo);
  jukebox_delay_delete(e->delay);
}

static void loresource_sound_set_initialize_direction_(
    loresource_sound_set_t* set,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format) {
  assert(set   != NULL);
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  typeof(set->direction)* d = &set->direction;

  d->compo = jukebox_composite_new(format, 1);

# define add_(NAME)  \
      jukebox_composite_add_effect(  \
        d->compo,  \
        (jukebox_effect_t*) set->items[LORESOURCE_SOUND_ID_##NAME].sound)
  add_(DYING);
# undef add_

  jukebox_composite_play(d->compo);
}
static void loresource_sound_set_deinitialize_direction_(
    loresource_sound_set_t* set) {
  assert(set != NULL);

  typeof(set->direction)* d = &set->direction;

  jukebox_composite_delete(d->compo);
}

void loresource_sound_set_initialize(
    loresource_sound_set_t* set,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format) {
  assert(set   != NULL);
  assert(mixer != NULL);
  assert(jukebox_format_valid(format));

  *set = (typeof(*set)) {0};

# define each_(NAME, name) do {  \
    jukebox_sound_buffer_t* buf = jukebox_sound_buffer_new_from_memory_mp3(  \
        format,  \
        loresource_sound_##name##_mp3_,  \
        sizeof(loresource_sound_##name##_mp3_));  \
    jukebox_sound_t* snd = jukebox_sound_new(buf, MAX_CONCURRENT_PLAYS_);  \
    set->items[LORESOURCE_SOUND_ID_##NAME] = (loresource_sound_t) {  \
      .buffer = buf,  \
      .sound  = snd,  \
    };  \
  } while (0)
    LORESOURCE_SOUND_ID_EACH(each_);
# undef each_

  set->root.compo = jukebox_composite_new(format, 4);

  loresource_sound_set_initialize_combat_(set, mixer, format);
  jukebox_composite_add_effect(
      set->root.compo, (jukebox_effect_t*) set->combat.compo);

  loresource_sound_set_initialize_env_(set, mixer, format);
  jukebox_composite_add_effect(
      set->root.compo, (jukebox_effect_t*) set->env.compo);

  loresource_sound_set_initialize_direction_(set, mixer, format);
  jukebox_composite_add_effect(
      set->root.compo, (jukebox_effect_t*) set->direction.compo);

  jukebox_amp_initialize(&set->root.amp, format);
  jukebox_composite_add_effect(set->root.compo, &set->root.amp.super);

  jukebox_amp_change_volume(&set->root.amp, 1, &rational(1, 1));
  jukebox_composite_play(set->root.compo);

  jukebox_mixer_add_effect(mixer, (jukebox_effect_t*) set->root.compo);
}

void loresource_sound_set_deinitialize(loresource_sound_set_t* set) {
  assert(set != NULL);

  jukebox_composite_delete(set->root.compo);
  jukebox_amp_deinitialize(&set->root.amp);

  loresource_sound_set_deinitialize_combat_(set);
  loresource_sound_set_deinitialize_env_(set);
  loresource_sound_set_deinitialize_direction_(set);

  for (size_t i = 0; i < LORESOURCE_SOUND_ID_COUNT; ++i) {
    jukebox_sound_delete(set->items[i].sound);
    jukebox_sound_buffer_delete(set->items[i].buffer);
  }
}

void loresource_sound_set_play(
    loresource_sound_set_t* set, loresource_sound_id_t id) {
  assert(set != NULL);

  jukebox_sound_play(set->items[id].sound);
}

void loresource_sound_set_change_master_volume(
    loresource_sound_set_t* set, float v, const rational_t* duration) {
  assert(set != NULL);
  assert(MATH_FLOAT_VALID(v));
  assert(rational_valid(duration));

  jukebox_amp_change_volume(&set->root.amp, v, duration);
}
