#pragma once

#include <stdbool.h>

#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loshader/posteffect.h"

typedef struct {
  const locommon_ticker_t* ticker;

  float  base_brightness;
  vec2_t chunk_winsz;

  uint64_t corruption_since;

  locommon_position_t pos;
  float               scale;

  float pixsort;
  loshader_posteffect_drawer_param_t posteffect;
} loplayer_camera_t;

void
loplayer_camera_initialize(
    loplayer_camera_t*       camera,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker
);

void
loplayer_camera_deinitialize(
    loplayer_camera_t* camera
);

void
loplayer_camera_build_matrix(
    const loplayer_camera_t* camera,
    mat4_t*                  m
);
