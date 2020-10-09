#pragma once

#include <stdbool.h>

#include "core/locommon/input.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"
#include "core/loworld/view.h"

#include "./combat.h"
#include "./event.h"
#include "./hud.h"
#include "./menu.h"
#include "./popup.h"

typedef struct {
  const locommon_ticker_t* ticker;
  const loworld_view_t*    view;

  loui_combat_t combat;
  loui_hud_t    hud;
  loui_popup_t  popup;
  loui_menu_t   menu;
  loui_event_t  event;

  bool pressed;
  bool menu_shown;
} loui_t;

void
loui_initialize(
    loui_t*                  ui,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loplayer_t*              player,
    const loworld_view_t*    view
);

void
loui_deinitialize(
    loui_t* ui
);

void
loui_update(
    loui_t*                 ui,
    const locommon_input_t* input
);

void
loui_draw(
    const loui_t* ui
);

bool
loui_is_grabbing_input(
    const loui_t* ui
);
