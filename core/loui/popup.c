#include "./popup.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"
#include "util/math/vector.h"

#include "core/locommon/easing.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loplayer/popup.h"
#include "core/loresource/set.h"
#include "core/loresource/text.h"
#include "core/loshader/set.h"

#define DURATION_ 4  /* [sec] */

#define HEAD_COLOR_ vec4(1, 1, 1, 1)

static void loui_popup_calculate_geometry_(
    loui_popup_t* popup, const locommon_screen_t* screen) {
  assert(popup  != NULL);
  assert(screen != NULL);

  typeof(popup->geo)* g = &popup->geo;

  locommon_screen_calc_pixels_from_inch(
      screen, &g->big_fontpx, &vec2(.6f, .6f));
  locommon_screen_calc_winpos_from_pixels(
      screen, &g->big_fontsz, &g->big_fontpx);
}

static glyphas_block_t* loui_popup_create_head_text_(
    loui_popup_t* popup, const locommon_screen_t* screen, const char* str) {
  assert(popup  != NULL);
  assert(screen != NULL);
  assert(str    != NULL);

  glyphas_block_t* b = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
      -popup->geo.big_fontpx.y,
      INT32_MAX,
      32);
  glyphas_block_add_characters(
      b,
      popup->font.serif.big,
      &HEAD_COLOR_,
      str,
      strlen(str));
  glyphas_block_scale(
      b, &vec2(2/screen->resolution.x, 2/screen->resolution.y));

  glyphas_block_set_origin(b, &vec2(.5f, -1));
  return b;
}

void loui_popup_initialize(
    loui_popup_t*            popup,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loplayer_t*              player) {
  assert(popup   != NULL);
  assert(res     != NULL);
  assert(shaders != NULL);
  assert(screen  != NULL);
  assert(ticker  != NULL);
  assert(player  != NULL);

  *popup = (typeof(*popup)) {
    .shaders = shaders,
    .ticker  = ticker,
    .player  = player,
  };
  loui_popup_calculate_geometry_(popup, screen);

  popup->font = (typeof(popup->font)) {
    .serif = {
      .big = glyphas_cache_new(
          shaders->tex.popup_text,
          &res->font.serif,
          popup->geo.big_fontpx.x,
          popup->geo.big_fontpx.y),
    },
  };

# define text_(v) loresource_text_get(res->lang, v)

  popup->head = (typeof(popup->head)) {
    .new_stance = loui_popup_create_head_text_(
        popup, screen, text_("popup_new_stance_head")),
  };

# undef text_
}

void loui_popup_deinitialize(loui_popup_t* popup) {
  assert(popup != NULL);

  glyphas_block_delete(popup->head.new_stance);
  glyphas_cache_delete(popup->font.serif.big);
}

void loui_popup_update(loui_popup_t* popup) {
  assert(popup != NULL);

  const float dt = popup->ticker->delta_f;
  locommon_easing_linear_float(&popup->alpha, 0, dt/DURATION_);
  if (popup->alpha > 0) return;

  const loplayer_popup_item_t* item =
      loplayer_popup_enqueue(&popup->player->popup);
  if (item == NULL) return;

  popup->item  = *item;
  popup->alpha = 1;
}

void loui_popup_draw(const loui_popup_t* popup) {
  assert(popup != NULL);

  if (popup->alpha <= 0) return;

  popup->shaders->drawer.popup_text.alpha = popup->alpha;

  switch (popup->item.type) {
  case LOPLAYER_POPUP_ITEM_TYPE_NEW_STANCE:
    loshader_popup_text_drawer_add_block(
        &popup->shaders->drawer.popup_text, popup->head.new_stance);
    break;
  }
}
