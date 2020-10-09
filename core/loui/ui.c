#include "./ui.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "core/locommon/easing.h"
#include "core/locommon/input.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"
#include "core/loworld/chunk.h"
#include "core/loworld/view.h"

#include "./combat.h"
#include "./event.h"
#include "./hud.h"
#include "./menu.h"

#define FADE_SPEED_ 4

void loui_initialize(
    loui_t*                  ui,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loplayer_t*              player,
    const loworld_view_t*    view) {
  assert(ui      != NULL);
  assert(res     != NULL);
  assert(shaders != NULL);
  assert(ticker  != NULL);
  assert(player  != NULL);
  assert(view    != NULL);
  assert(locommon_screen_valid(screen));

  *ui = (typeof(*ui)) {
    .ticker = ticker,
    .view   = view,
  };

  loui_combat_initialize(
      &ui->combat,
      shaders,
      ticker,
      player);
  loui_hud_initialize(
      &ui->hud,
      res,
      shaders,
      screen,
      ticker,
      player);
  loui_popup_initialize(
      &ui->popup,
      res,
      shaders,
      screen,
      ticker,
      player);
  loui_menu_initialize(
      &ui->menu,
      res,
      shaders,
      screen,
      ticker,
      player);
  loui_event_initialize(
      &ui->event,
      res,
      shaders,
      screen,
      ticker,
      player);
}

void loui_deinitialize(loui_t* ui) {
  assert(ui != NULL);

  loui_event_deinitialize(&ui->event);
  loui_menu_deinitialize(&ui->menu);
  loui_popup_deinitialize(&ui->popup);
  loui_hud_deinitialize(&ui->hud);
  loui_combat_deinitialize(&ui->combat);
}

void loui_update(loui_t* ui, const locommon_input_t* input) {
  assert(ui    != NULL);
  assert(input != NULL);

  const bool  pressed = input->buttons & LOCOMMON_INPUT_BUTTON_MENU;
  const float dt      = ui->ticker->delta_f;

  if (pressed && !ui->pressed) ui->menu_shown = !ui->menu_shown;
  ui->pressed = pressed;
  locommon_easing_smooth_float(
      &ui->menu.alpha, !!ui->menu_shown, dt*FADE_SPEED_);

  const bool hud = !ui->menu_shown;
  locommon_easing_smooth_float(
      &ui->hud.alpha, !!hud, dt*FADE_SPEED_);

  const loworld_chunk_t* chunk = loworld_view_get_looking_chunk(ui->view);
  ui->hud.current_biome = chunk->biome;

  loui_combat_update(&ui->combat);
  loui_hud_update(&ui->hud);
  loui_popup_update(&ui->popup);
  loui_menu_update(&ui->menu, ui->menu_shown? input: NULL);
  loui_event_update(&ui->event);
}

void loui_draw(const loui_t* ui) {
  assert(ui != NULL);

  loui_combat_draw(&ui->combat);
  loui_hud_draw(&ui->hud);
  loui_popup_draw(&ui->popup);
  loui_menu_draw(&ui->menu);
  loui_event_draw(&ui->event);
}

bool loui_is_grabbing_input(const loui_t* ui) {
  assert(ui != NULL);

  return ui->menu_shown;
}
