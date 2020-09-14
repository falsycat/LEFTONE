#pragma once

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "core/locommon/input.h"

bool
app_event_handle(
    locommon_input_t* input,
    const SDL_Event*  e
);
