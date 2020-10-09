#pragma once

#include "core/locommon/input.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"

typedef enum {
  LOPLAYER_CONTROLLER_STATE_NONE,
  LOPLAYER_CONTROLLER_STATE_WALK_LEFT,
  LOPLAYER_CONTROLLER_STATE_WALK_RIGHT,
  LOPLAYER_CONTROLLER_STATE_SPRINT_LEFT,
  LOPLAYER_CONTROLLER_STATE_SPRINT_RIGHT,
  LOPLAYER_CONTROLLER_STATE_DODGE_FORWARD,
  LOPLAYER_CONTROLLER_STATE_DODGE_LEFT,
  LOPLAYER_CONTROLLER_STATE_DODGE_RIGHT,
  LOPLAYER_CONTROLLER_STATE_JUMP,
  LOPLAYER_CONTROLLER_STATE_GUARD,
  LOPLAYER_CONTROLLER_STATE_SHOOT,
} loplayer_controller_state_t;

typedef struct {
  const locommon_ticker_t* ticker;

  loplayer_controller_state_t state;
  locommon_position_t         cursor;

  bool     jump;
  bool     sprint;
  uint64_t last_sprint_start;
} loplayer_controller_t;

void
loplayer_controller_initialize(
    loplayer_controller_t*   controller,
    const locommon_ticker_t* ticker
);

void
loplayer_controller_deinitialize(
    loplayer_controller_t* controller
);

void
loplayer_controller_handle_input(
    loplayer_controller_t*     controller,
    const locommon_input_t*    input,  /* NULLABLE */
    const locommon_position_t* cursor  /* NULLABLE */
);
