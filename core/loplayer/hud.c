#include "./hud.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "util/conv/charcode.h"
#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"
#include "util/math/algorithm.h"
#include "util/memory/memory.h"

#include "core/locommon/easing.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loresource/set.h"
#include "core/loresource/text.h"
#include "core/loshader/hud_bar.h"
#include "core/loshader/set.h"

#include "./entity.h"
#include "./event.h"
#include "./status.h"

typedef struct {
  float alpha;
  float remain;

  glyphas_block_t* text;
} loplayer_hud_effect_t;

struct loplayer_hud_t {
  /* injected deps */
  loshader_set_t*          shaders;
  const locommon_ticker_t* ticker;
  const loplayer_event_t*  event;
  const loplayer_status_t* status;
  const loplayer_entity_t* entity;

  /* owned objects */
  struct {
    glyphas_cache_t* sans;
    glyphas_cache_t* serif;
  } font;

  /* immutable params */
  struct {
    vec2_t fontsz_normal_px;
    vec2_t fontsz_normal;
    vec2_t fontsz_small_px;
    vec2_t fontsz_small;

    vec2_t padding;

    vec2_t madness_bar_pos;
    vec2_t madness_bar_size;

    vec2_t faith_bar_pos;
    vec2_t faith_bar_size;

    vec2_t effect_bar_pos;
    vec2_t effect_bar_size;
  } geometry;

  /* mutable params */
  float alpha;

  float prev_madness;
  float prev_faith;

  bool shown;

  glyphas_block_t* biome_text;

  union {
    struct {
      loplayer_hud_effect_t curse;
      loplayer_hud_effect_t amnesia;
      loplayer_hud_effect_t lost;
    };
    loplayer_hud_effect_t array[3];
  } effects;
};

static void loplayer_hud_calculate_geometry_(loplayer_hud_t* hud) {
  assert(hud != NULL);

  const vec2_t* dpi  = &hud->shaders->dpi;
  const vec2_t* reso = &hud->shaders->resolution;

  typeof(hud->geometry)* geo = &hud->geometry;

# define px_to_disp_(v) vec2((v).x/reso->x*2, (v).y/reso->y*2)

  geo->fontsz_normal_px = *dpi;
  vec2_muleq(&geo->fontsz_normal_px, .4f);

  geo->fontsz_normal = px_to_disp_(geo->fontsz_normal_px);

  geo->fontsz_small_px = *dpi;
  vec2_muleq(&geo->fontsz_small_px, .3f);

  geo->fontsz_small = px_to_disp_(geo->fontsz_small_px);

  geo->padding = *dpi;
  vec2_muleq(&geo->padding, .4f);
  geo->padding = px_to_disp_(geo->padding);

  geo->madness_bar_size = vec2(.5f, .1f*dpi->y);
  geo->madness_bar_size.y /= reso->y/2;

  geo->madness_bar_pos = vec2(
      -1 + geo->padding.x + geo->madness_bar_size.x,
      1 - geo->padding.y - geo->madness_bar_size.y);

  geo->faith_bar_size = vec2(.4f, .05f*dpi->y);
  geo->faith_bar_size.y /= reso->y/2;

  geo->faith_bar_pos = vec2(
      -1 + geo->padding.x + geo->faith_bar_size.x,
      1 - geo->padding.y - geo->madness_bar_size.y*2 - geo->faith_bar_size.y);

  geo->effect_bar_size = vec2(3*dpi->x/reso->x, 4/reso->y);

  geo->effect_bar_pos = vec2(1-geo->padding.x, -1+geo->padding.y);
  vec2_subeq(&geo->effect_bar_pos, &geo->effect_bar_size);

# undef px_to_disp_
}

static glyphas_block_t* loplayer_hud_create_effect_text_block_(
    const loplayer_hud_t* hud, const char* text) {
  assert(hud != NULL);

  glyphas_block_t* block = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
      -hud->geometry.fontsz_normal_px.y,
      INT32_MAX,
      32);

  static const vec4_t white = vec4(1, 1, 1, 1);
  glyphas_block_add_characters(
      block, hud->font.sans, &white, text, strlen(text));

  const vec2_t scale = vec2(
      2/hud->shaders->resolution.x, 2/hud->shaders->resolution.y);
  glyphas_block_scale(block, &scale);

  return block;
}

static void loplayer_hud_initialize_effect_text_(
    loplayer_hud_t* hud, loresource_set_t* res) {
  assert(hud != NULL);

# define init_effect_text_(name)  \
      hud->effects.name.text = loplayer_hud_create_effect_text_block_(  \
          hud, loresource_text_get(res->lang, "effect_"#name))

  init_effect_text_(curse);
  init_effect_text_(amnesia);
  init_effect_text_(lost);

# undef init_effect_
}

static void loplayer_hud_update_bars_(loplayer_hud_t* hud) {
  assert(hud != NULL);

  const float d = hud->ticker->delta_f;

  locommon_easing_smooth_float(
      &hud->prev_madness, hud->status->recipient.madness, d*2);
  locommon_easing_smooth_float(
      &hud->prev_faith, hud->status->recipient.faith, d*2);
}

static void loplayer_hud_update_lasting_effect_(
    loplayer_hud_t*                         hud,
    loplayer_hud_effect_t*                  e,
    const loeffect_generic_lasting_param_t* p) {
  assert(hud != NULL);
  assert(e   != NULL);
  assert(p   != NULL);

  const uint64_t end = p->begin + p->duration;
  const uint64_t t   = hud->ticker->time;
  if (p->duration == 0 || end <= t || p->begin > t) {
    e->remain = 0;
    return;
  }
  e->remain = 1 - (t - p->begin)*1.f/p->duration;
}

static void loplayer_hud_update_effects_(loplayer_hud_t* hud) {
  assert(hud != NULL);

  const float d = hud->ticker->delta_f;

  loplayer_hud_update_lasting_effect_(
      hud, &hud->effects.curse, &hud->status->recipient.effects.curse);
  loplayer_hud_update_lasting_effect_(
      hud, &hud->effects.amnesia, &hud->status->recipient.effects.amnesia);
  hud->effects.lost.remain =
      hud->status->recipient.faith > 0? 0: (1-hud->status->recipient.madness);

  static const size_t len =
      sizeof(hud->effects.array)/sizeof(hud->effects.array[0]);
  for (size_t i = 0; i < len; ++i) {
    loplayer_hud_effect_t* e = &hud->effects.array[i];
    locommon_easing_linear_float(&e->alpha, !!(e->remain > 0), d*2);
  }
}

static void loplayer_hud_draw_bars_(const loplayer_hud_t* hud) {
  assert(hud != NULL);

  const typeof(hud->geometry)* geo = &hud->geometry;

  const loshader_hud_bar_drawer_instance_t madness = {
    .pos        = geo->madness_bar_pos,
    .size       = geo->madness_bar_size,
    .bgcolor    = vec4(0, 0, 0, 1*hud->alpha),
    .fgcolor    = vec4(1, 1, 1, .9f*hud->alpha),
    .value      = MATH_CLAMP(hud->status->recipient.madness, 0, 1),
    .prev_value = MATH_CLAMP(hud->prev_madness, 0, 1),
  };
  loshader_hud_bar_drawer_add_instance(hud->shaders->drawer.hud_bar, &madness);

  const loshader_hud_bar_drawer_instance_t faith = {
    .pos        = geo->faith_bar_pos,
    .size       = geo->faith_bar_size,
    .bgcolor    = vec4(0, 0, 0, 1*hud->alpha),
    .fgcolor    = vec4(.9f, .9f, .9f, .9f*hud->alpha),
    .value      = MATH_CLAMP(hud->status->recipient.faith, 0, 1),
    .prev_value = MATH_CLAMP(hud->prev_faith, 0, 1),
  };
  loshader_hud_bar_drawer_add_instance(hud->shaders->drawer.hud_bar, &faith);
}

static void loplayer_hud_draw_effects_(const loplayer_hud_t* hud) {
  assert(hud != NULL);

  const typeof(hud->geometry)* geo = &hud->geometry;

  const float lineheight = hud->geometry.fontsz_normal.y;
  float h = 0;

  static const size_t len =
      sizeof(hud->effects.array)/sizeof(hud->effects.array[0]);
  for (size_t i = 0; i < len; ++i) {
    const loplayer_hud_effect_t* e = &hud->effects.array[i];
    if (e->alpha == 0) continue;

    const float y = h*lineheight + geo->effect_bar_pos.y;

    static const vec2_t origin = vec2(1, -1);
    glyphas_block_set_origin(e->text, &origin);

    const vec2_t trans = vec2(1-geo->padding.x, y);
    glyphas_block_translate(e->text, &trans);
    glyphas_block_set_alpha(e->text, e->alpha);

    loshader_hud_text_drawer_add_block(
        &hud->shaders->drawer.hud_text, e->text);

    const loshader_hud_bar_drawer_instance_t instance = {
      .pos        = vec2(geo->effect_bar_pos.x, y),
      .size       = vec2(-geo->effect_bar_size.x, geo->effect_bar_size.y),
      .bgcolor    = vec4(1, 1, 1, .2f*hud->alpha),
      .fgcolor    = vec4(1, 1, 1, .8f*hud->alpha),
      .value      = MATH_CLAMP(e->remain, 0, 1),
      .prev_value = MATH_CLAMP(e->remain, 0, 1),
    };
    loshader_hud_bar_drawer_add_instance(
        hud->shaders->drawer.hud_bar, &instance);

    h += e->alpha * e->alpha * (3-2*e->alpha);
  }
}

loplayer_hud_t* loplayer_hud_new(
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    const loplayer_event_t*  event,
    const loplayer_status_t* status,
    const loplayer_entity_t* entity) {
  assert(res     != NULL);
  assert(shaders != NULL);
  assert(ticker  != NULL);
  assert(event   != NULL);
  assert(status  != NULL);
  assert(entity  != NULL);

  loplayer_hud_t* hud = memory_new(sizeof(*hud));
  *hud = (typeof(*hud)) {
    .shaders = shaders,
    .ticker  = ticker,
    .event   = event,
    .status  = status,
    .entity  = entity,
  };
  loplayer_hud_calculate_geometry_(hud);

  hud->font = (typeof(hud->font)) {
    .sans = glyphas_cache_new(
        shaders->tex.hud_text,
        &res->font.sans,
        hud->geometry.fontsz_normal_px.x,
        hud->geometry.fontsz_normal_px.y),
    .serif = glyphas_cache_new(
        shaders->tex.hud_text,
        &res->font.serif,
        hud->geometry.fontsz_small_px.x,
        hud->geometry.fontsz_small_px.y),
  };

  hud->biome_text = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
      -hud->geometry.fontsz_normal_px.y,
      INT32_MAX,
      32);

  loplayer_hud_initialize_effect_text_(hud, res);

  return hud;
}

void loplayer_hud_delete(loplayer_hud_t* hud) {
  if (hud == NULL) return;

  static const size_t len =
      sizeof(hud->effects.array) / sizeof(hud->effects.array[0]);
  for (size_t i = 0; i < len; ++i) {
    glyphas_block_delete(hud->effects.array[i].text);
  }

  glyphas_block_delete(hud->biome_text);

  glyphas_cache_delete(hud->font.sans);
  glyphas_cache_delete(hud->font.serif);

  memory_delete(hud);
}

void loplayer_hud_show(loplayer_hud_t* hud) {
  assert(hud != NULL);

  hud->shown = true;
}

void loplayer_hud_hide(loplayer_hud_t* hud) {
  assert(hud != NULL);

  hud->shown = false;
}

void loplayer_hud_set_biome_text(loplayer_hud_t* hud, const char* text) {
  assert(hud != NULL);

  glyphas_block_clear(hud->biome_text);

  static const vec4_t white = vec4(1, 1, 1, 1);
  glyphas_block_add_characters(
      hud->biome_text, hud->font.serif, &white, text, strlen(text));

  glyphas_block_set_origin(hud->biome_text, &vec2(0, -1));

  const vec2_t scale = vec2(
      2/hud->shaders->resolution.x, 2/hud->shaders->resolution.y);
  glyphas_block_scale(hud->biome_text, &scale);

  const typeof(hud->geometry)* geo = &hud->geometry;
  glyphas_block_translate(
      hud->biome_text, &vec2(-1+geo->padding.x, -1+geo->padding.y));

  glyphas_block_make_glitched(hud->biome_text, hud->ticker->time);
}

void loplayer_hud_update(loplayer_hud_t* hud) {
  assert(hud != NULL);

  bool shown = hud->shown;

  const loplayer_event_param_t* e = loplayer_event_get_param(hud->event);
  if (e != NULL) {
    shown = shown && !e->hide_hud;
  }

  const float dt = hud->ticker->delta_f;
  locommon_easing_smooth_float(&hud->alpha, !!shown, dt*2);

  loplayer_hud_update_bars_(hud);
  loplayer_hud_update_effects_(hud);
}

void loplayer_hud_draw_ui(const loplayer_hud_t* hud) {
  assert(hud != NULL);

  hud->shaders->drawer.hud_text.alpha = hud->alpha;

  loplayer_hud_draw_bars_(hud);
  loplayer_hud_draw_effects_(hud);

  loshader_hud_text_drawer_add_block(
      &hud->shaders->drawer.hud_text, hud->biome_text);
}
