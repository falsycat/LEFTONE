#include "./event.h"

#include <assert.h>
#include <stddef.h>

#include "util/math/vector.h"
#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"

#include "core/locommon/easing.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/event.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loresource/text.h"
#include "core/loshader/set.h"

#define LINE_MAX_CHARS_ 256

#define CINESCOPE_SIZE_ .23f
#define LINE_COLOR_ vec4(1, 1, 1, 1)

static void loui_event_update_line_(loui_event_t* ev) {
  assert(ev != NULL);

  if (ev->player->event.ctx.line.last_update <= ev->last_line_update) {
    return;
  }
  ev->last_line_update = ev->player->event.ctx.line.last_update;

  const char* id = ev->player->event.ctx.line.text_id;
  const char* s  = id == NULL? "": loresource_text_get(ev->res->lang, id);

  glyphas_block_clear(ev->line);
  glyphas_block_add_characters(
      ev->line, ev->font, &LINE_COLOR_, s, strlen(s));

  glyphas_block_scale(
      ev->line,
      &vec2(2/ev->screen->resolution.x, 2/ev->screen->resolution.y));
  glyphas_block_set_origin(ev->line, &vec2(.5f, -.5f));

  vec2_t sz, offset;
  glyphas_block_calculate_geometry(ev->line, &sz, &offset);

  const float y = -1 + CINESCOPE_SIZE_/2 + sz.y/2;
  glyphas_block_translate(ev->line, &vec2(0, y));
}

void loui_event_initialize(
    loui_event_t*            ev,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loplayer_t*              player) {
  assert(ev      != NULL);
  assert(res     != NULL);
  assert(shaders != NULL);
  assert(screen  != NULL);
  assert(ticker  != NULL);
  assert(player  != NULL);

  vec2_t fontpx;
  locommon_screen_calc_pixels_from_inch(screen, &fontpx, &vec2(.15f, .15f));

  *ev = (typeof(*ev)) {
    .res     = res,
    .shaders = shaders,
    .screen  = screen,
    .ticker  = ticker,
    .player  = player,

    .font = glyphas_cache_new(
        shaders->tex.event_line,
        &res->font.serif,
        fontpx.x,
        fontpx.y),
    .line = glyphas_block_new(
        GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
        -fontpx.y,
        INT32_MAX,
        LINE_MAX_CHARS_),
  };
}

void loui_event_deinitialize(loui_event_t* ev) {
  assert(ev != NULL);

  glyphas_block_delete(ev->line);
  glyphas_cache_delete(ev->font);
}

void loui_event_update(loui_event_t* ev) {
  assert(ev != NULL);

  loui_event_update_line_(ev);

  const float dt = ev->ticker->delta_f;
  locommon_easing_smooth_float(
      &ev->cinescope, ev->player->event.ctx.cinescope*CINESCOPE_SIZE_, dt);
}

void loui_event_draw(const loui_event_t* ev) {
  assert(ev != NULL);

  loshader_event_line_drawer_add_block(
      &ev->shaders->drawer.event_line, ev->line);

  loshader_cinescope_drawer_set_param(
      &ev->shaders->drawer.cinescope,
      &(loshader_cinescope_drawer_param_t) {
        .size  = ev->cinescope,
        .color = vec4(0, 0, 0, 1),
      });
}
