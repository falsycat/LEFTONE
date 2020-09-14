#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "util/math/matrix.h"
#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loshader/cinescope.h"
#include "core/loshader/posteffect.h"
#include "core/loshader/set.h"

#include "./entity.h"
#include "./event.h"
#include "./status.h"

typedef enum {
  LOPLAYER_CAMERA_STATE_DEFAULT,
  LOPLAYER_CAMERA_STATE_COMBAT,
  LOPLAYER_CAMERA_STATE_DEAD,
  LOPLAYER_CAMERA_STATE_MENU,
} loplayer_camera_state_t;

typedef struct {
  /* injected deps */
  loshader_set_t*          shaders;
  const locommon_ticker_t* ticker;
  const loplayer_event_t*  event;
  const loplayer_status_t* status;
  const loplayer_entity_t* entity;

  /* immutable params */
  vec2_t display_chunksz;

  /* read-only mutable params */
  locommon_position_t pos;
  mat4_t              matrix;

  float scale;
  float pixsort;
  loshader_posteffect_drawer_param_t pe;
  loshader_cinescope_drawer_param_t  cinesco;

  /* public params */
  loplayer_camera_state_t state;
  float                   brightness;

} loplayer_camera_t;

const char*
loplayer_camera_state_stringify(
    loplayer_camera_state_t state
);

bool
loplayer_camera_state_unstringify(
    loplayer_camera_state_t* state,
    const char*              str,
    size_t                   len
);

void
loplayer_camera_initialize(
    loplayer_camera_t*       camera,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    const loplayer_event_t*  event,
    const loplayer_status_t* status,
    const loplayer_entity_t* entity,
    const mat4_t*            proj
);

void
loplayer_camera_deinitialize(
    loplayer_camera_t* camera
);

void
loplayer_camera_update(
    loplayer_camera_t* camera
);

void
loplayer_camera_draw(
    const loplayer_camera_t* camera
);

void
loplayer_camera_pack(
    const loplayer_camera_t* camera,
    msgpack_packer* packer
);

bool
loplayer_camera_unpack(
    loplayer_camera_t*    camera,
    const msgpack_object* packer
);
