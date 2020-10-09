#include "./hud.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"
#include "util/math/vector.h"

#include "core/lochara/base.h"
#include "core/locommon/easing.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loresource/text.h"
#include "core/loshader/set.h"

#define BAR_SPEED_ 5

#define STATUS_MAX_WIDTH_  .6f
#define STATUS_FAITH_SIZE_ .8f

#define EFFECT_TEXT_COLOR_  vec4(1, 1, 1, 1)
#define EFFECT_FADE_SPEED_  2
#define EFFECT_LINE_HEIGHT_ 1.2f

#define BIOME_TEXT_COLOR_ vec4(1, 1, 1, 1)
#define BIOME_TEXT_SEED_  29  /* = prime number */

static void loui_hud_calculate_geometry_(
    loui_hud_t* hud, const locommon_screen_t* screen) {
  assert(hud != NULL);
  assert(locommon_screen_valid(screen));

  typeof(hud->geo)* g = &hud->geo;

# define inch2winpos_(winpos, inch)  \
    locommon_screen_calc_winpos_from_inch(screen, winpos, inch)

  inch2winpos_(&g->padding, &vec2(.3f, .3f));

  inch2winpos_(&g->madness_sz, &vec2(3.f, .12f));
  if (g->madness_sz.x > STATUS_MAX_WIDTH_) {
    vec2_muleq(&g->madness_sz, STATUS_MAX_WIDTH_/g->madness_sz.x);
  }
  vec2_mul(&g->faith_sz, &g->madness_sz, .5f);

# undef inch2winpos_

  g->madness_pos = vec2(-1, 1);
  g->madness_pos.x += g->madness_sz.x + g->padding.x;
  g->madness_pos.y -= g->madness_sz.y + g->padding.y;

  g->faith_pos = vec2(-1, 1);
  g->faith_pos.x += g->padding.x + g->faith_sz.x;
  g->faith_pos.y -= g->padding.y + g->madness_sz.y*2 + g->faith_sz.y;

  locommon_screen_calc_pixels_from_inch(
      screen, &g->effect_fontpx, &vec2(.4f, .4f));
  locommon_screen_calc_winpos_from_pixels(
      screen, &g->effect_fontsz, &g->effect_fontpx);

  g->effect_sz.x = 3*screen->dpi.x/screen->resolution.x;
  g->effect_sz.y = 4/screen->resolution.y;

  g->effect_pos = vec2(1-g->padding.x, -1+g->padding.y);
  vec2_subeq(&g->effect_pos, &g->effect_sz);

  locommon_screen_calc_pixels_from_inch(
      screen, &g->biome_fontpx, &vec2(.3f, .3f));
  locommon_screen_calc_winpos_from_pixels(
      screen, &g->biome_fontsz, &g->biome_fontpx);

  vec2_add(&g->biome_pos, &vec2(-1, -1), &g->padding);
}

static void loui_hud_draw_status_bars_(const loui_hud_t* hud) {
  assert(hud != NULL);

  loshader_hud_bar_drawer_add_instance(
      &hud->shaders->drawer.hud_bar,
      &(loshader_hud_bar_drawer_instance_t) {
        .pos  = hud->geo.madness_pos,
        .size = hud->geo.madness_sz,
        .bgcolor = vec4(0, 0, 0, hud->alpha),
        .fgcolor = vec4(1, 1, 1, .9f*hud->alpha),

        .value      = hud->player->entity->param.recipient.madness,
        .prev_value = hud->prev_madness,
      });
  loshader_hud_bar_drawer_add_instance(
      &hud->shaders->drawer.hud_bar,
      &(loshader_hud_bar_drawer_instance_t) {
        .pos  = hud->geo.faith_pos,
        .size = hud->geo.faith_sz,
        .bgcolor = vec4(0, 0, 0, hud->alpha),
        .fgcolor = vec4(.9f, .9f, .9f, .9f*hud->alpha),

        .value      = hud->player->entity->param.recipient.faith,
        .prev_value = hud->prev_faith,
      });
}

static void loui_hud_initialize_effect_(
    loui_hud_t*              hud,
    loui_hud_effect_t*       effect,
    const locommon_screen_t* screen,
    const char*              name) {
  assert(hud    != NULL);
  assert(effect != NULL);
  assert(name   != NULL);
  assert(locommon_screen_valid(screen));

  *effect = (typeof(*effect)) {
    .text = glyphas_block_new(
        GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
        -hud->geo.effect_fontpx.y,
        INT32_MAX,
        32),
  };
  glyphas_block_add_characters(
      effect->text, hud->font.effect, &EFFECT_TEXT_COLOR_, name, strlen(name));

  const vec2_t reso = screen->resolution;
  glyphas_block_scale(effect->text, &vec2(2/reso.x, 2/reso.y));
}

static void loui_hud_deinitialize_effect_(loui_hud_effect_t* effect) {
  assert(effect != NULL);

  glyphas_block_delete(effect->text);
}

static void loui_hud_update_effect_bars_(loui_hud_t* hud) {
  assert(hud != NULL);

  const loeffect_recipient_t* r = &hud->player->entity->param.recipient;
  loui_hud_effect_set_t*      e = &hud->effects.set;

  const uint64_t t  = hud->ticker->time;
  const float    dt = hud->ticker->delta_f;

# define lasting_effect_(n) do {  \
    const bool active =  \
        r->effects.n.start <= t &&  \
        t < r->effects.n.start + r->effects.n.duration;  \
    locommon_easing_smooth_float(  \
        &e->n.alpha, !!active, dt*EFFECT_FADE_SPEED_);  \
    if (active) {  \
      e->n.value = (t - r->effects.n.start)*1.f / r->effects.n.duration;  \
    }  \
  } while (0)

  lasting_effect_(amnesia);
  lasting_effect_(curse);

# undef lasting_effect_
}

static void loui_hud_draw_effect_bars_(const loui_hud_t* hud) {
  assert(hud != NULL);

  const typeof(hud->geo)* g = &hud->geo;

  float h = 0;
  for (size_t i = 0; i < LOUI_HUD_EFFECT_COUNT; ++i) {
    const loui_hud_effect_t* e = &hud->effects.arr[i];
    if (e->alpha == 0) continue;

    const float a = e->alpha * hud->alpha;
    const float y = h*g->effect_fontsz.y + g->effect_pos.y;

    glyphas_block_set_origin(e->text, &vec2(1, -1));
    glyphas_block_translate(e->text, &vec2(1-g->padding.x, y));
    glyphas_block_set_alpha(e->text, powf(a, 2));

    loshader_hud_text_drawer_add_block(
        &hud->shaders->drawer.hud_text, e->text);

    loshader_hud_bar_drawer_add_instance(
        &hud->shaders->drawer.hud_bar,
        &(loshader_hud_bar_drawer_instance_t) {
          .pos        = vec2(g->effect_pos.x, y),
          .size       = vec2(-g->effect_sz.x, g->effect_sz.y),
          .bgcolor    = vec4(1, 1, 1, .2f*a),
          .fgcolor    = vec4(1, 1, 1, .8f*a),
          .value      = e->value,
          .prev_value = e->value,
        });
    h += a*a*(3-2*a)*EFFECT_LINE_HEIGHT_;
  }
}

static glyphas_block_t* loui_hud_create_biome_text_(
    loui_hud_t* hud, const locommon_screen_t* screen, const char* text) {
  assert(hud  != NULL);
  assert(text != NULL);
  assert(locommon_screen_valid(screen));

  glyphas_block_t* b = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
      -hud->geo.biome_fontpx.y,
      INT32_MAX,
      32);
  glyphas_block_add_characters(
      b, hud->font.biome, &BIOME_TEXT_COLOR_, text, strlen(text));

  const vec2_t reso = screen->resolution;
  glyphas_block_scale(b, &vec2(2/reso.x, 2/reso.y));

  glyphas_block_set_origin(b, &vec2(0, -1));
  glyphas_block_translate(b, &hud->geo.biome_pos);

  glyphas_block_make_glitched(b, text[0]*BIOME_TEXT_SEED_);
  return b;
}

static void loui_hud_draw_biome_text_(const loui_hud_t* hud) {
  assert(hud != NULL);

  loshader_hud_text_drawer_add_block(
      &hud->shaders->drawer.hud_text, hud->biomes[hud->current_biome]);
}

void loui_hud_initialize(
    loui_hud_t*              hud,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loplayer_t*              player) {
  assert(hud     != NULL);
  assert(res     != NULL);
  assert(shaders != NULL);
  assert(ticker  != NULL);
  assert(player  != NULL);
  assert(locommon_screen_valid(screen));

  *hud = (typeof(*hud)) {
    .shaders = shaders,
    .ticker  = ticker,
    .player  = player,
  };
  loui_hud_calculate_geometry_(hud, screen);

  hud->font = (typeof(hud->font)) {
    .effect = glyphas_cache_new(
        shaders->tex.hud_text,
        &res->font.sans,
        hud->geo.effect_fontpx.x,
        hud->geo.effect_fontpx.y),
    .biome = glyphas_cache_new(
        shaders->tex.hud_text,
        &res->font.serif,
        hud->geo.biome_fontpx.x,
        hud->geo.biome_fontpx.y),
  };

# define text_(name) loresource_text_get(res->lang, name)

  loui_hud_effect_set_t* e = &hud->effects.set;
  loui_hud_initialize_effect_(
      hud, &e->amnesia, screen, text_("effect_amnesia"));
  loui_hud_initialize_effect_(
      hud, &e->curse, screen, text_("effect_curse"));

# define each_(NAME, name) do { \
    hud->biomes[LOWORLD_CHUNK_BIOME_##NAME] =  \
        loui_hud_create_biome_text_(hud, screen, text_("biome_"#name));  \
  } while (0)
  LOWORLD_CHUNK_BIOME_EACH(each_);
# undef each_

# undef text_
}

void loui_hud_deinitialize(loui_hud_t* hud) {
  assert(hud != NULL);

  for (size_t i = 0; i < LOUI_HUD_EFFECT_COUNT; ++i) {
    loui_hud_deinitialize_effect_(&hud->effects.arr[i]);
  }
  for (size_t i = 0; i < LOWORLD_CHUNK_BIOME_COUNT; ++i) {
    glyphas_block_delete(hud->biomes[i]);
  }
  glyphas_cache_delete(hud->font.effect);
  glyphas_cache_delete(hud->font.biome);
}

void loui_hud_update(loui_hud_t* hud) {
  assert(hud != NULL);

  const float dt = hud->ticker->delta_f;

  locommon_easing_smooth_float(
      &hud->prev_madness,
      hud->player->entity->param.recipient.madness,
      dt*BAR_SPEED_);
  locommon_easing_smooth_float(
      &hud->prev_faith, hud->player->entity->param.recipient.faith,
      dt*BAR_SPEED_);

  loui_hud_update_effect_bars_(hud);
}

void loui_hud_draw(const loui_hud_t* hud) {
  assert(hud != NULL);

  hud->shaders->drawer.hud_text.alpha = hud->alpha;

  loui_hud_draw_status_bars_(hud);
  loui_hud_draw_effect_bars_(hud);
  loui_hud_draw_biome_text_(hud);
}
