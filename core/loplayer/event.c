#include "./event.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "core/locommon/position.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

struct loplayer_event_t {
  loplayer_event_param_t param;
      /* convertible between loplayer_event_t* and loplayer_event_param_t* */

  /* injected deps */
  loresource_set_t* res;
  loshader_set_t*   shaders;

  /* owned objects */
  glyphas_cache_t* font;
  glyphas_block_t* text;

  /* immutable params */
  struct {
    vec2_t fontsz;
  } geometry;
};

static void loplayer_event_calculate_geometry_(loplayer_event_t* event) {
  assert(event != NULL);

  typeof(event->geometry)* geo = &event->geometry;

  geo->fontsz = event->shaders->dpi;
  vec2_muleq(&geo->fontsz, .15f);
}

loplayer_event_t* loplayer_event_new(
    loresource_set_t* res, loshader_set_t* shaders) {
  assert(res     != NULL);
  assert(shaders != NULL);

  loplayer_event_t* event = memory_new(sizeof(*event));
  *event = (typeof(*event)) {
    .res     = res,
    .shaders = shaders,
  };
  loplayer_event_calculate_geometry_(event);

  event->font = glyphas_cache_new(
      shaders->tex.event_line,
      &res->font.serif,
      event->geometry.fontsz.x,
      event->geometry.fontsz.y);

  event->text = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
      -event->geometry.fontsz.y,
      INT32_MAX,
      256);

  return event;
}

void loplayer_event_delete(loplayer_event_t* event) {
  if (event == NULL) return;

  glyphas_cache_delete(event->font);
  glyphas_block_delete(event->text);

  memory_delete(event);
}

loplayer_event_param_t* loplayer_event_take_control(
    loplayer_event_t* event, loentity_id_t id) {
  assert(event != NULL);

  if (event->param.controlled) return NULL;

  event->param = (typeof(event->param)) {
    .controlled    = true,
    .controlled_by = id,
  };
  return &event->param;
}

void loplayer_event_abort(loplayer_event_t* event) {
  assert(event != NULL);

  if (!event->param.controlled) return;

  loplayer_event_param_release_control(&event->param);
}

void loplayer_event_draw(const loplayer_event_t* event) {
  assert(event != NULL);

  if (!event->param.controlled) return;

  loshader_event_line_drawer_add_block(
      &event->shaders->drawer.event_line, event->text);
}

const loplayer_event_param_t* loplayer_event_get_param(
    const loplayer_event_t* event) {
  assert(event != NULL);

  if (!event->param.controlled) return NULL;

  return &event->param;
}

void loplayer_event_param_set_line(
    loplayer_event_param_t* param, const char* str, size_t len) {
  assert(param != NULL);

  loplayer_event_t* event = (typeof(event)) param;

  glyphas_block_clear(event->text);

  static const vec4_t white = vec4(1, 1, 1, 1);
  glyphas_block_add_characters(event->text, event->font, &white, str, len);

  static const vec2_t center = vec2(.5f, -.5f);
  glyphas_block_set_origin(event->text, &center);

  const vec2_t scale = vec2(
      2/event->shaders->resolution.x, 2/event->shaders->resolution.y);
  glyphas_block_scale(event->text, &scale);

  static const vec2_t trans = vec2(0, -.85f);
  glyphas_block_translate(event->text, &trans);
}

void loplayer_event_param_release_control(loplayer_event_param_t* param) {
  assert(param != NULL);
  assert(param->controlled);

  loplayer_event_param_set_line(param, "", 0);
  param->controlled = false;
}
