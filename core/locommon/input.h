#pragma once

#include <stdint.h>

#include "util/math/vector.h"

typedef enum {
  LOCOMMON_INPUT_BUTTON_LEFT  = 1 << 0,
  LOCOMMON_INPUT_BUTTON_RIGHT = 1 << 1,
  LOCOMMON_INPUT_BUTTON_DODGE = 1 << 2,
  LOCOMMON_INPUT_BUTTON_JUMP  = 1 << 3,
  LOCOMMON_INPUT_BUTTON_GUARD = 1 << 4,
  LOCOMMON_INPUT_BUTTON_SHOOT = 1 << 5,
  LOCOMMON_INPUT_BUTTON_MENU  = 1 << 6,

  LOCOMMON_INPUT_BUTTON_OK     = LOCOMMON_INPUT_BUTTON_SHOOT,
  LOCOMMON_INPUT_BUTTON_CANCEL = LOCOMMON_INPUT_BUTTON_MENU,
} locommon_input_button_t;

typedef uint16_t locommon_input_buttons_t;

typedef struct {
  locommon_input_buttons_t buttons;

  vec2_t resolution;  /* in pixels */
  vec2_t dpi;
  vec2_t cursor;    /* -1~1 */
} locommon_input_t;
