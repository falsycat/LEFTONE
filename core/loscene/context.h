#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "util/flasy/flasy.h"
#include "util/glyphas/context.h"
#include "util/jukebox/mixer.h"
#include "util/math/vector.h"

#include "core/locommon/input.h"
#include "core/locommon/ticker.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./param.h"
#include "./scene.h"

typedef struct {
  flasy_t*          flasy;
  glyphas_context_t glyphas;
  jukebox_mixer_t*  mixer;

  locommon_screen_t screen;
  locommon_ticker_t ticker;

  loresource_set_t resources;
  loshader_set_t   shaders;

  loscene_t* scene;

  loscene_param_t param;
} loscene_context_t;

void
loscene_context_initialize(
    loscene_context_t*     ctx,
    const loscene_param_t* param
);

void
loscene_context_deinitialize(
    loscene_context_t* ctx
);

bool
loscene_context_update(
    loscene_context_t*       ctx,
    const locommon_input_t*  input,
    uint64_t                 uptime
);

void
loscene_context_draw(
    loscene_context_t* ctx
);
