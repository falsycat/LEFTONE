#pragma once

#include <stdint.h>

#include "util/jukebox/amp.h"
#include "util/jukebox/composite.h"
#include "util/jukebox/delay.h"
#include "util/jukebox/format.h"
#include "util/jukebox/mixer.h"
#include "util/jukebox/sound.h"
#include "util/math/rational.h"

typedef enum {
  /* BENUM BEGIN loresource_sound_id */
  LORESOURCE_SOUND_ID_BOMB,
  LORESOURCE_SOUND_ID_DAMAGE,
  LORESOURCE_SOUND_ID_DODGE,
  LORESOURCE_SOUND_ID_DYING,
  LORESOURCE_SOUND_ID_ENEMY_SHOOT,
  LORESOURCE_SOUND_ID_ENEMY_TRIGGER,
  LORESOURCE_SOUND_ID_GUARD,
  LORESOURCE_SOUND_ID_PLAYER_SHOOT,
  LORESOURCE_SOUND_ID_PLAYER_TRIGGER,
  LORESOURCE_SOUND_ID_REFLECTION,
  LORESOURCE_SOUND_ID_TOUCH_GATE,
  /* BENUM END */
} loresource_sound_id_t;

#include "core/loresource/benum/sound.h"

typedef struct {
  jukebox_sound_buffer_t* buffer;
  jukebox_sound_t*        sound;
} loresource_sound_t;

typedef struct {
  loresource_sound_t items[LORESOURCE_SOUND_ID_COUNT];

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
} loresource_sound_set_t;

void
loresource_sound_set_initialize(
    loresource_sound_set_t* set,
    jukebox_mixer_t*        mixer,
    const jukebox_format_t* format
);

void
loresource_sound_set_deinitialize(
    loresource_sound_set_t* set
);

void
loresource_sound_set_play(
    loresource_sound_set_t* set,
    loresource_sound_id_t   id
);

void
loresource_sound_set_change_master_volume(
    loresource_sound_set_t* set,
    float                   v,
    const rational_t*       duration
);
