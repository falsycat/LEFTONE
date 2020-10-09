#pragma once

#include <stdbool.h>

#include "./context.h"
#include "./scene.h"

loscene_t*  /* OWNERSHIP */
loscene_game_new(
    loscene_context_t* ctx,
    bool               load
);
