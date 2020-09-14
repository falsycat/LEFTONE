#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/loentity/entity.h"
#include "core/loresource/music.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

struct loplayer_event_t;
typedef struct loplayer_event_t loplayer_event_t;

typedef struct {
  bool controlled;
      /* You can return the control by assigning false. */

  loentity_id_t controlled_by;

  locommon_position_t area_pos;
  vec2_t              area_size;

  bool cinescope;
  bool hide_hud;

  loresource_music_player_t* music;
} loplayer_event_param_t;

loplayer_event_t*  /* OWNERSHIP */
loplayer_event_new(
    loresource_set_t* res,
    loshader_set_t*   shaders
);

void
loplayer_event_delete(
    loplayer_event_t* event  /* OWNERSHIP*/
);

loplayer_event_param_t*  /* NULLABLE */
loplayer_event_take_control(
    loplayer_event_t* event,
    loentity_id_t     id
);

void
loplayer_event_abort(
    loplayer_event_t* event
);

void
loplayer_event_draw(
    const loplayer_event_t* event
);

const loplayer_event_param_t*  /* NULLABLE */
loplayer_event_get_param(
    const loplayer_event_t* event
);

void
loplayer_event_param_set_line(
    loplayer_event_param_t* param,
    const char*             str,
    size_t                  len
);

void
loplayer_event_param_release_control(
    loplayer_event_param_t* param
);
