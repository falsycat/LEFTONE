#include "./title.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <GL/glew.h>
#include <msgpack.h>
#include <msgpack/fbuffer.h>

#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"
#include "util/jukebox/amp.h"
#include "util/jukebox/decoder.h"
#include "util/math/matrix.h"
#include "util/math/rational.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "core/locommon/input.h"
#include "core/locommon/position.h"
#include "core/loresource/music.h"
#include "core/loresource/text.h"
#include "core/loshader/backwall.h"
#include "core/loshader/fog.h"
#include "core/loshader/menu_text.h"

#include "./context.h"
#include "./game.h"
#include "./param.h"
#include "./scene.h"

typedef enum {
  LOSCENE_TITLE_STATE_EXPECT_BUTTON_RELEASE,
  LOSCENE_TITLE_STATE_EXPECT_BUTTON_PRESS,

  LOSCENE_TITLE_STATE_FADING_TO_GAME,
  LOSCENE_TITLE_STATE_FADING_TO_EXIT,
} loscene_title_state_t;

typedef struct {
  loscene_t header;

  loscene_context_t* ctx;

  struct {
    vec2_t fontsz_large;
    vec2_t fontsz_small;

    vec2_t fontpx_large;
    vec2_t fontpx_small;
  } geo;

  struct {
    glyphas_cache_t* large;
    glyphas_cache_t* small;
  } font;

  struct {
    glyphas_block_t* title;
    glyphas_block_t* author;
    glyphas_block_t* buttons;
  } text;

  loresource_music_t* music;

  loscene_title_state_t state;
  uint64_t              since;

  float fade;

} loscene_title_t;

static void loscene_title_calculate_geometry_(loscene_title_t* s) {
  assert(s != NULL);

  locommon_screen_calc_pixels_from_inch(
      &s->ctx->screen, &s->geo.fontpx_large, &vec2(.8f, .8f));
  locommon_screen_calc_pixels_from_inch(
      &s->ctx->screen, &s->geo.fontpx_small, &vec2(.14f, .14f));
  locommon_screen_calc_winpos_from_pixels(
      &s->ctx->screen, &s->geo.fontsz_large, &s->geo.fontpx_large);
  locommon_screen_calc_winpos_from_pixels(
      &s->ctx->screen, &s->geo.fontsz_small, &s->geo.fontpx_small);
}

static void loscene_title_create_text_block_(loscene_title_t* s) {
  assert(s != NULL);

  const char* title   =
      loresource_text_get(s->ctx->resources.lang, "app_name");
  const char* author  =
      loresource_text_get(s->ctx->resources.lang, "title_authors");
  const char* buttons =
      loresource_text_get(s->ctx->resources.lang, "title_buttons");

  static const vec4_t color = vec4(1, 1, 1, 1);

  s->text = (typeof(s->text)) {
    .title = glyphas_block_new(
        GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
        -s->geo.fontpx_large.y,
        INT32_MAX,
        16),
    .author = glyphas_block_new(
        GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
        -s->geo.fontpx_small.y,
        INT32_MAX,
        256),
    .buttons = glyphas_block_new(
        GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
        -s->geo.fontpx_small.y,
        INT32_MAX,
        16),
  };

  glyphas_block_add_characters(
      s->text.title, s->font.large, &color, title, strlen(title));
  glyphas_block_add_characters(
      s->text.author, s->font.small, &color, author, strlen(author));
  glyphas_block_add_characters(
      s->text.buttons, s->font.small, &color, buttons, strlen(buttons));

  const vec2_t scale = vec2(
      2/s->ctx->screen.resolution.x, 2/s->ctx->screen.resolution.y);
  glyphas_block_scale(s->text.title,  &scale);
  glyphas_block_scale(s->text.author, &scale);
  glyphas_block_scale(s->text.buttons, &scale);

  glyphas_block_set_origin(s->text.title,  &vec2(.5f, -.5f));
  glyphas_block_set_origin(s->text.author, &vec2(.5f, 0));
  glyphas_block_set_origin(s->text.buttons, &vec2(.5f, -1.f));

  glyphas_block_translate(
      s->text.author, &vec2(0, -s->geo.fontsz_large.y*.7f));
  glyphas_block_translate(
      s->text.buttons, &vec2(0, -.9f));
}

static void loscene_title_update_uniblock_(loscene_title_t* s) {
  assert(s != NULL);

  static const uint64_t camspeed = 5000;

  const uint64_t chunk = s->ctx->ticker.time/camspeed;
  const float    fract = s->ctx->ticker.time%camspeed*1.f/camspeed;

  loshader_uniblock_update_param(
      &s->ctx->shaders.uniblock,
      &(loshader_uniblock_param_t) {
        .proj = mat4_identity(),
        .cam  = mat4_identity(),
        .pos  = locommon_position(chunk, 0, vec2(fract, 0)),
        .time = s->ctx->ticker.time%60000/1000.0f,
      });
}

static void loscene_title_delete_(loscene_t* scene) {
  if (scene == NULL) return;

  loscene_title_t* s = (typeof(s)) scene;

  glyphas_block_delete(s->text.title);
  glyphas_block_delete(s->text.author);
  glyphas_block_delete(s->text.buttons);

  glyphas_cache_delete(s->font.large);
  glyphas_cache_delete(s->font.small);

  memory_delete(s);
}

static loscene_t* loscene_title_update_(
    loscene_t* scene, const locommon_input_t* input) {
  assert(scene != NULL);
  assert(input != NULL);

  static const uint64_t fadedur = 1500;

  loscene_title_t* s = (typeof(s)) scene;

  if (s->music == NULL) {
    s->music = loresource_music_set_get(
        &s->ctx->resources.music, LORESOURCE_MUSIC_ID_TITLE);
    jukebox_decoder_play(s->music->decoder, &rational(0, 1), true);
    jukebox_amp_change_volume(&s->music->amp, 1, &rational(1, 1));
  }

  switch (s->state) {
  case LOSCENE_TITLE_STATE_EXPECT_BUTTON_RELEASE:
    s->fade = 1 - (s->ctx->ticker.time - s->since)*1.f / fadedur;
    if (s->fade < 0) s->fade = 0;
    if (input->buttons == 0 && s->fade <= 0) {
      s->state = LOSCENE_TITLE_STATE_EXPECT_BUTTON_PRESS;
    }
    break;
  case LOSCENE_TITLE_STATE_EXPECT_BUTTON_PRESS:
    if (input->buttons & LOCOMMON_INPUT_BUTTON_OK) {
      jukebox_amp_change_volume(&s->music->amp, 0, &rational(fadedur, 1000));
      jukebox_decoder_stop_after(s->music->decoder, &rational(fadedur, 1000));

      s->state = LOSCENE_TITLE_STATE_FADING_TO_GAME;
      s->since = s->ctx->ticker.time;
    } else if (input->buttons & LOCOMMON_INPUT_BUTTON_MENU) {
      jukebox_amp_change_volume(&s->music->amp, 0, &rational(fadedur, 1000));
      jukebox_decoder_stop_after(s->music->decoder, &rational(fadedur, 1000));

      s->state = LOSCENE_TITLE_STATE_FADING_TO_EXIT;
      s->since = s->ctx->ticker.time;
    }
    break;
  case LOSCENE_TITLE_STATE_FADING_TO_GAME:
    if (s->since + fadedur < s->ctx->ticker.time) {
      return loscene_game_new(s->ctx, true  /* = load data */);
    } else {
      s->fade = (s->ctx->ticker.time - s->since)*1.f/fadedur;
    }
    break;
  case LOSCENE_TITLE_STATE_FADING_TO_EXIT:
    if (s->since + fadedur < s->ctx->ticker.time) {
      return NULL;
    } else {
      s->fade = (s->ctx->ticker.time - s->since)*1.f/fadedur;
    }
    break;
  }
  return scene;
}

static void loscene_title_draw_(loscene_t* scene) {
  assert(scene != NULL);

  loscene_title_t* s = (typeof(s)) scene;
  loscene_title_update_uniblock_(s);

  loshader_set_clear_all(&s->ctx->shaders);

  loshader_backwall_drawer_set_param(
      &s->ctx->shaders.drawer.backwall,
      &(loshader_backwall_drawer_param_t) {
        .type       = LOSHADER_BACKWALL_TYPE_JAIL,
        .transition = 1,
      });

  loshader_fog_drawer_set_param(
      &s->ctx->shaders.drawer.fog,
      &(loshader_fog_drawer_param_t) {
        .type       = LOSHADER_FOG_TYPE_WHITE_CLOUD,
        .transition = 1,
      });

  s->ctx->shaders.drawer.pixsort.intensity = 0;

  loshader_posteffect_drawer_set_param(
      &s->ctx->shaders.drawer.posteffect,
      &(loshader_posteffect_drawer_param_t) {
        .blur_whole         = 1,
        .distortion_radial  = .05f,
        .distortion_amnesia = .1f,
        .fade_radial        = .5f + s->fade*.3f,
        .brightness_whole   = s->ctx->param.brightness/1000.f,
      });

  s->ctx->shaders.drawer.menu_text.alpha = 1;
  loshader_menu_text_drawer_add_block(
      &s->ctx->shaders.drawer.menu_text, s->text.title);
  loshader_menu_text_drawer_add_block(
      &s->ctx->shaders.drawer.menu_text, s->text.author);
  loshader_menu_text_drawer_add_block(
      &s->ctx->shaders.drawer.menu_text, s->text.buttons);

  loshader_cinescope_drawer_set_param(
      &s->ctx->shaders.drawer.cinescope,
      &(loshader_cinescope_drawer_param_t) {0});

  loshader_set_draw_all(&s->ctx->shaders);
}

loscene_t* loscene_title_new(loscene_context_t* ctx) {
  assert(ctx != NULL);

  loshader_set_drop_cache(&ctx->shaders);

  loscene_title_t* s = memory_new(sizeof(*s));
  *s = (typeof(*s)) {
    .header = {
      .vtable = {
        .delete = loscene_title_delete_,
        .update = loscene_title_update_,
        .draw   = loscene_title_draw_,
      },
    },
    .ctx = ctx,

    .state = LOSCENE_TITLE_STATE_EXPECT_BUTTON_RELEASE,
    .since = ctx->ticker.time,
  };
  loscene_title_calculate_geometry_(s);

  s->font = (typeof(s->font)) {
    .large = glyphas_cache_new(
        s->ctx->shaders.tex.menu_text,
        &s->ctx->resources.font.sans,
        s->geo.fontpx_large.x,
        s->geo.fontpx_large.y),
    .small = glyphas_cache_new(
        s->ctx->shaders.tex.menu_text,
        &s->ctx->resources.font.sans,
        s->geo.fontpx_small.x,
        s->geo.fontpx_small.y),
  };
  loscene_title_create_text_block_(s);
  return &s->header;
}
