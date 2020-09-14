#pragma once

#include <stdbool.h>

#include "core/locommon/input.h"
#include "core/locommon/position.h"

typedef enum {
  LOPLAYER_CONTROLLER_MOVEMENT_NONE,
  LOPLAYER_CONTROLLER_MOVEMENT_JUMP,
  LOPLAYER_CONTROLLER_MOVEMENT_WALK_LEFT,
  LOPLAYER_CONTROLLER_MOVEMENT_WALK_RIGHT,
  LOPLAYER_CONTROLLER_MOVEMENT_DASH_LEFT,
  LOPLAYER_CONTROLLER_MOVEMENT_DASH_RIGHT,
} loplayer_controller_movement_t;

typedef enum {
  LOPLAYER_CONTROLLER_ACTION_NONE,
  LOPLAYER_CONTROLLER_ACTION_ATTACK,
  LOPLAYER_CONTROLLER_ACTION_GUARD,
  LOPLAYER_CONTROLLER_ACTION_UNGUARD,
  LOPLAYER_CONTROLLER_ACTION_DODGE,
  LOPLAYER_CONTROLLER_ACTION_MENU,
} loplayer_controller_action_t;

typedef struct {
  locommon_position_t looking;
  vec2_t              cursor;  /* display coordinate (-1~1) */

  loplayer_controller_movement_t movement;
  loplayer_controller_action_t   action;

  locommon_input_t prev;
} loplayer_controller_t;

void
loplayer_controller_initialize(
    loplayer_controller_t* controller
);

void
loplayer_controller_deinitialize(
    loplayer_controller_t* controller
);

void
loplayer_controller_update(
    loplayer_controller_t*     controller,
    const locommon_input_t*    input,
    const locommon_position_t* cursor
);
