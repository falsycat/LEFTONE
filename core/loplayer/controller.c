#include "./controller.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "core/locommon/input.h"
#include "core/locommon/position.h"

void loplayer_controller_initialize(loplayer_controller_t* controller) {
  assert(controller != NULL);

  *controller = (typeof(*controller)) {0};
}

void loplayer_controller_deinitialize(loplayer_controller_t* controller) {
  assert(controller != NULL);

}

void loplayer_controller_update(
    loplayer_controller_t*     controller,
    const locommon_input_t*    input,
    const locommon_position_t* cursor) {
  assert(controller != NULL);
  assert(input      != NULL);
  assert(locommon_position_valid(cursor));

  controller->looking = *cursor;
  controller->cursor  = input->cursor;

  controller->movement = LOPLAYER_CONTROLLER_MOVEMENT_NONE;
  controller->action   = LOPLAYER_CONTROLLER_ACTION_NONE;

  const bool prev_jump =
      controller->prev.buttons & LOCOMMON_INPUT_BUTTON_JUMP;
  const bool prev_guarding =
      controller->prev.buttons & LOCOMMON_INPUT_BUTTON_GUARD;
  const bool prev_dash =
      controller->prev.buttons & LOCOMMON_INPUT_BUTTON_DASH;
  const bool prev_menu =
      controller->prev.buttons & LOCOMMON_INPUT_BUTTON_MENU;

  if (input->buttons & LOCOMMON_INPUT_BUTTON_JUMP && !prev_jump) {
    controller->movement = LOPLAYER_CONTROLLER_MOVEMENT_JUMP;

  } else if (input->buttons & LOCOMMON_INPUT_BUTTON_LEFT) {
    controller->movement = LOPLAYER_CONTROLLER_MOVEMENT_WALK_LEFT;
    if (input->buttons & LOCOMMON_INPUT_BUTTON_DASH) {
      controller->movement = LOPLAYER_CONTROLLER_MOVEMENT_DASH_LEFT;
    }

  } else if (input->buttons & LOCOMMON_INPUT_BUTTON_RIGHT) {
    controller->movement = LOPLAYER_CONTROLLER_MOVEMENT_WALK_RIGHT;
    if (input->buttons & LOCOMMON_INPUT_BUTTON_DASH) {
      controller->movement = LOPLAYER_CONTROLLER_MOVEMENT_DASH_RIGHT;
    }
  }

  if (input->buttons & LOCOMMON_INPUT_BUTTON_ATTACK) {
    controller->action = LOPLAYER_CONTROLLER_ACTION_ATTACK;
  }

  if (input->buttons & LOCOMMON_INPUT_BUTTON_GUARD) {
    if (!prev_guarding) controller->action = LOPLAYER_CONTROLLER_ACTION_GUARD;
  } else {
    if (prev_guarding) controller->action = LOPLAYER_CONTROLLER_ACTION_UNGUARD;
  }

  if (input->buttons & LOCOMMON_INPUT_BUTTON_DASH && !prev_dash) {
    controller->action = LOPLAYER_CONTROLLER_ACTION_DODGE;
  }

  if (input->buttons & LOCOMMON_INPUT_BUTTON_MENU && !prev_menu) {
    controller->action = LOPLAYER_CONTROLLER_ACTION_MENU;
  }

  controller->prev = *input;
}
