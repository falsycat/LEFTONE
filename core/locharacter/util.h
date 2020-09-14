#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "core/loentity/entity.h"
#include "core/loresource/music.h"
#include "core/loplayer/event.h"

#include "./base.h"

typedef struct {
  loresource_music_player_t* music;
  locharacter_base_t*        owner;

  uint64_t duration;

  loplayer_event_param_t* param;
  uint64_t                start_time;
} locharacter_event_holder_t;

void
locharacter_event_holder_initialize(
    locharacter_event_holder_t* holder,
    loresource_music_player_t*  music,
    locharacter_base_t*         owner,
    uint64_t                    duration,
    uint64_t                    start_time
);

void
locharacter_event_holder_deinitialize(
    locharacter_event_holder_t* holder
);

bool
locharacter_event_holder_take_control(
    locharacter_event_holder_t* holder
);

void
locharacter_event_holder_release_control(
    locharacter_event_holder_t* holder
);

bool  /* false: event was aborted by others */
locharacter_event_holder_update(
    locharacter_event_holder_t* holder
);

bool
locharacter_event_holder_has_control(
    const locharacter_event_holder_t* holder
);
