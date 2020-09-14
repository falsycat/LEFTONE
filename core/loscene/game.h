#pragma once

#include "core/locommon/ticker.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./param.h"
#include "./scene.h"

loscene_t*  /* OWNERSHIP */
loscene_game_new(
    const loscene_param_t*   param,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    bool                     load
);
