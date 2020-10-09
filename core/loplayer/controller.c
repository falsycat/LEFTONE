#include "./controller.h"

#include <assert.h>
#include <stddef.h>

#include "core/locommon/input.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"

#define DODGE_PRESS_MAX_DURATION_ 300

void loplayer_controller_initialize(
    loplayer_controller_t* controller, const locommon_ticker_t* ticker) {
  assert(controller != NULL);
  assert(ticker     != NULL);

  *controller = (typeof(*controller)) {
    .ticker = ticker,
  };
}

void loplayer_controller_deinitialize(loplayer_controller_t* controller) {
  assert(controller != NULL);

}

void loplayer_controller_handle_input(
    loplayer_controller_t*     controller,
    const locommon_input_t*    input,
    const locommon_position_t* cursor) {
  assert(controller != NULL);

  if (locommon_position_valid(cursor)) {
    controller->cursor = *cursor;
  }

  controller->state = LOPLAYER_CONTROLLER_STATE_NONE;
  if (input == NULL) return;

  if (input->buttons & LOCOMMON_INPUT_BUTTON_LEFT) {
    controller->state = LOPLAYER_CONTROLLER_STATE_WALK_LEFT;
  }
  if (input->buttons & LOCOMMON_INPUT_BUTTON_RIGHT) {
    controller->state = LOPLAYER_CONTROLLER_STATE_WALK_RIGHT;
  }

  if (input->buttons & LOCOMMON_INPUT_BUTTON_DODGE) {
    if (!controller->sprint) {
      controller->sprint            = true;
      controller->last_sprint_start = controller->ticker->time;
    }
    if (input->buttons & LOCOMMON_INPUT_BUTTON_LEFT) {
      controller->state = LOPLAYER_CONTROLLER_STATE_SPRINT_LEFT;
    } else if (input->buttons & LOCOMMON_INPUT_BUTTON_RIGHT) {
      controller->state = LOPLAYER_CONTROLLER_STATE_SPRINT_RIGHT;
    }
  } else {
    if (controller->sprint) {
      controller->sprint = false;

      assert(controller->ticker->time >= controller->last_sprint_start);
      const size_t t =
          controller->ticker->time - controller->last_sprint_start;
      if (t < DODGE_PRESS_MAX_DURATION_) {
        if (input->buttons & LOCOMMON_INPUT_BUTTON_LEFT) {
          controller->state = LOPLAYER_CONTROLLER_STATE_DODGE_LEFT;
        } else if (input->buttons & LOCOMMON_INPUT_BUTTON_RIGHT) {
          controller->state = LOPLAYER_CONTROLLER_STATE_DODGE_RIGHT;
        } else {
          controller->state = LOPLAYER_CONTROLLER_STATE_DODGE_FORWARD;
        }
      }
    }
  }

  if (input->buttons & LOCOMMON_INPUT_BUTTON_JUMP) {
    if (!controller->jump) {
      controller->jump  = true;
      controller->state = LOPLAYER_CONTROLLER_STATE_JUMP;
    }
  } else {
    controller->jump = false;
  }

  if (input->buttons & LOCOMMON_INPUT_BUTTON_GUARD) {
    controller->state = LOPLAYER_CONTROLLER_STATE_GUARD;
  }
  if (input->buttons & LOCOMMON_INPUT_BUTTON_SHOOT) {
    controller->state = LOPLAYER_CONTROLLER_STATE_SHOOT;
  }
}
