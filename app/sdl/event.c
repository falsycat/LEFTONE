#include "./event.h"

#include <assert.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "core/locommon/input.h"

#define APP_EVENT_GET_BUTTON_BIT_FROM_KEY(k) (  \
    (k) == SDLK_a?      LOCOMMON_INPUT_BUTTON_LEFT:  \
    (k) == SDLK_d?      LOCOMMON_INPUT_BUTTON_RIGHT:  \
    (k) == SDLK_w?      LOCOMMON_INPUT_BUTTON_UP:  \
    (k) == SDLK_s?      LOCOMMON_INPUT_BUTTON_DOWN:  \
    (k) == SDLK_SPACE?  LOCOMMON_INPUT_BUTTON_JUMP:  \
    (k) == SDLK_LSHIFT? LOCOMMON_INPUT_BUTTON_DASH:  \
    (k) == SDLK_ESCAPE? LOCOMMON_INPUT_BUTTON_MENU:  \
    0)

#define APP_EVENT_GET_BUTTON_BIT_FROM_MOUSE(m) (  \
    (m) == SDL_BUTTON_LEFT?  LOCOMMON_INPUT_BUTTON_ATTACK:  \
    (m) == SDL_BUTTON_RIGHT? LOCOMMON_INPUT_BUTTON_GUARD:   \
    0)

bool app_event_handle(locommon_input_t* input, const SDL_Event* e) {
  assert(input != NULL);
  assert(e     != NULL);

  switch (e->type) {
  case SDL_MOUSEMOTION:
    input->cursor = vec2(
        e->motion.x/input->resolution.x, e->motion.y/input->resolution.y);
    input->cursor.x = input->cursor.x*2 - 1;
    input->cursor.y = 1 - input->cursor.y*2;
    break;
  case SDL_MOUSEBUTTONDOWN:
    input->buttons |= APP_EVENT_GET_BUTTON_BIT_FROM_MOUSE(e->button.button);
    break;
  case SDL_MOUSEBUTTONUP:
    input->buttons &= ~APP_EVENT_GET_BUTTON_BIT_FROM_MOUSE(e->button.button);
    break;
  case SDL_KEYDOWN:
    input->buttons |= APP_EVENT_GET_BUTTON_BIT_FROM_KEY(e->key.keysym.sym);
    break;
  case SDL_KEYUP:
    input->buttons &= ~APP_EVENT_GET_BUTTON_BIT_FROM_KEY(e->key.keysym.sym);
    break;
  case SDL_QUIT:
    return false;
  }
  return true;
}
