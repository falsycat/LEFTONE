#include "./title.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <GL/glew.h>
#include <msgpack.h>
#include <msgpack/fbuffer.h>

#include "util/chaos/xorshift.h"
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
#include "core/loresource/set.h"
#include "core/loresource/text.h"
#include "core/loshader/backwall.h"
#include "core/loshader/fog.h"
#include "core/loshader/menu_text.h"
#include "core/loshader/set.h"

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

  const loscene_param_t*   param;
  loresource_set_t*        res;
  loshader_set_t*          shaders;
  const locommon_ticker_t* ticker;

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

  loresource_music_player_t* music;

  loscene_title_state_t state;
  uint64_t              since;

  float fade;

} loscene_title_t;

static void loscene_title_calculate_geometry_(loscene_title_t* s) {
  assert(s != NULL);

  const vec2_t* dpi  = &s->shaders->dpi;
  const vec2_t* reso = &s->shaders->resolution;

  s->geo.fontpx_large = *dpi;
  vec2_muleq(&s->geo.fontpx_large, .8f);

  s->geo.fontpx_small = *dpi;
  vec2_muleq(&s->geo.fontpx_small, .14f);

# define px_to_disp_(v) vec2((v).x/reso->x*2, (v).y/reso->y*2)

  s->geo.fontsz_large = px_to_disp_(s->geo.fontpx_large);
  s->geo.fontsz_small = px_to_disp_(s->geo.fontpx_small);

# undef px_to_disp_
}

static void loscene_title_create_text_block_(loscene_title_t* s) {
  assert(s != NULL);

  const char* title   = loresource_text_get(s->res->lang, "app_name");
  const char* author  = loresource_text_get(s->res->lang, "title_authors");
  const char* buttons = loresource_text_get(s->res->lang, "title_buttons");

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
      2/s->shaders->resolution.x, 2/s->shaders->resolution.y);
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

  const uint64_t chunk = s->ticker->time/camspeed;
  const float    fract = s->ticker->time%camspeed*1.f/camspeed;

  const loshader_uniblock_param_t p = {
    .proj = mat4_identity(),
    .cam  = mat4_identity(),
    .pos  = locommon_position(chunk, 0, vec2(fract, 0)),
    .time = s->ticker->time%60000/1000.0f,
  };
  loshader_uniblock_update_param(s->shaders->uniblock, &p);
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
    s->music = &s->res->music.title;
    jukebox_decoder_play(s->music->decoder, &rational(0, 1), true);
    jukebox_amp_change_volume(&s->music->amp, 1, &rational(1, 1));
  }

  switch (s->state) {
  case LOSCENE_TITLE_STATE_EXPECT_BUTTON_RELEASE:
    s->fade = 1 - (s->ticker->time - s->since)*1.f / fadedur;
    if (s->fade < 0) s->fade = 0;
    if (input->buttons == 0 && s->fade <= 0) {
      s->state = LOSCENE_TITLE_STATE_EXPECT_BUTTON_PRESS;
    }
    break;
  case LOSCENE_TITLE_STATE_EXPECT_BUTTON_PRESS:
    if (input->buttons & LOCOMMON_INPUT_BUTTON_ATTACK) {
      jukebox_amp_change_volume(&s->music->amp, 0, &rational(fadedur, 1000));
      jukebox_decoder_stop_after(s->music->decoder, &rational(fadedur, 1000));

      s->state = LOSCENE_TITLE_STATE_FADING_TO_GAME;
      s->since = s->ticker->time;
    } else if (input->buttons & LOCOMMON_INPUT_BUTTON_MENU) {
      jukebox_amp_change_volume(&s->music->amp, 0, &rational(fadedur, 1000));
      jukebox_decoder_stop_after(s->music->decoder, &rational(fadedur, 1000));

      s->state = LOSCENE_TITLE_STATE_FADING_TO_EXIT;
      s->since = s->ticker->time;
    }
    break;
  case LOSCENE_TITLE_STATE_FADING_TO_GAME:
    if (s->since + fadedur < s->ticker->time) {
      return loscene_game_new(
          s->param, s->res, s->shaders, s->ticker, true  /* = load data */);
    } else {
      s->fade = (s->ticker->time - s->since)*1.f/fadedur;
    }
    break;
  case LOSCENE_TITLE_STATE_FADING_TO_EXIT:
    if (s->since + fadedur < s->ticker->time) {
      return NULL;
    } else {
      s->fade = (s->ticker->time - s->since)*1.f/fadedur;
    }
    break;
  }
  return scene;
}

static void loscene_title_draw_(loscene_t* scene) {
  assert(scene != NULL);

  loscene_title_t* s = (typeof(s)) scene;
  loscene_title_update_uniblock_(s);

  loshader_set_clear_all(s->shaders);

  loshader_backwall_drawer_set_param(
      s->shaders->drawer.backwall,
      &(loshader_backwall_drawer_param_t) {
        .type       = LOSHADER_BACKWALL_TYPE_JAIL,
        .transition = 1,
      });

  loshader_fog_drawer_set_param(
      s->shaders->drawer.fog,
      &(loshader_fog_drawer_param_t) {
        .type       = LOSHADER_FOG_TYPE_WHITE_CLOUD,
        .transition = 1,
      });

  loshader_pixsort_drawer_set_intensity(s->shaders->drawer.pixsort, 0);

  loshader_posteffect_drawer_set_param(
      s->shaders->drawer.posteffect,
      &(loshader_posteffect_drawer_param_t) {
        .whole_blur           = 1,
        .radial_displacement  = .05f,
        .amnesia_displacement = .1f,
        .radial_fade          = .5f + s->fade*.3f,
        .brightness           = s->param->brightness/1000.f,
      });

  s->shaders->drawer.menu_text.alpha = 1;
  loshader_menu_text_drawer_add_block(
      &s->shaders->drawer.menu_text, s->text.title);
  loshader_menu_text_drawer_add_block(
      &s->shaders->drawer.menu_text, s->text.author);
  loshader_menu_text_drawer_add_block(
      &s->shaders->drawer.menu_text, s->text.buttons);

  loshader_cinescope_drawer_set_param(
      s->shaders->drawer.cinescope,
      &(loshader_cinescope_drawer_param_t) {0});

  loshader_set_draw_all(s->shaders);
}

loscene_t* loscene_title_new(
    const loscene_param_t*   param,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker) {
  assert(param   != NULL);
  assert(res     != NULL);
  assert(shaders != NULL);
  assert(ticker  != NULL);

  loshader_set_drop_cache(shaders);

  loscene_title_t* scene = memory_new(sizeof(*scene));
  *scene = (typeof(*scene)) {
    .header = {
      .vtable = {
        .delete = loscene_title_delete_,
        .update = loscene_title_update_,
        .draw   = loscene_title_draw_,
      },
    },
    .param   = param,
    .res     = res,
    .shaders = shaders,
    .ticker  = ticker,

    .state = LOSCENE_TITLE_STATE_EXPECT_BUTTON_RELEASE,
    .since = ticker->time,
  };
  loscene_title_calculate_geometry_(scene);

  scene->font = (typeof(scene->font)) {
    .large = glyphas_cache_new(
        shaders->tex.menu_text,
        &res->font.sans,
        scene->geo.fontpx_large.x,
        scene->geo.fontpx_large.y),
    .small = glyphas_cache_new(
        shaders->tex.menu_text,
        &res->font.sans,
        scene->geo.fontpx_small.x,
        scene->geo.fontpx_small.y),
  };
  loscene_title_create_text_block_(scene);
  return &scene->header;
}
