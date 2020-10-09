#include "./menu.h"

#include <assert.h>
#include <stddef.h>

#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"
#include "util/math/constant.h"
#include "util/math/vector.h"

#include "core/locommon/easing.h"
#include "core/locommon/input.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loplayer/stance.h"
#include "core/loresource/set.h"
#include "core/loresource/text.h"
#include "core/loshader/set.h"

#define FADE_SPEED_ 2

#define STANCE_TEXT_LINEHEIGHT_ 1.5f
#define STANCE_TEXT_MAX_WIDTH_  3.5f  /* inch */

#define STANCE_NAME_RESERVE_   32
#define STANCE_DESC_RESERVE_   256
#define STANCE_NOTE_RESERVE_   256

#define STANCE_NAME_COLOR_ vec4(1, 1, 1, 1)
#define STANCE_DESC_COLOR_ vec4(1, 1, 1, 1)
#define STANCE_NOTE_COLOR_ vec4(.8f, .8f, 0, 1)

#define STANCE_ICON_HIGHLIGHT_SPEED_ 5
#define STANCE_ICON_HIGHLIGHT_SIZE_  .2f

static void loui_menu_calc_geometry_(
    loui_menu_t* menu, const locommon_screen_t* screen) {
  assert(menu != NULL);
  assert(locommon_screen_valid(screen));

  typeof(menu->geo)* g = &menu->geo;

# define calc_font_geo_(name, sz)  do {  \
    locommon_screen_calc_pixels_from_inch(  \
        screen, &g->name##_fontpx, sz);  \
    locommon_screen_calc_winpos_from_pixels(  \
        screen, &g->name##_fontsz, &g->big_fontpx);  \
  } while (0)

  calc_font_geo_(big,    &vec2(.20f, .20f));
  calc_font_geo_(normal, &vec2(.16f, .16f));
  calc_font_geo_(small,  &vec2(.12f, .12f));

# undef calc_font_geo_

  locommon_screen_calc_winpos_from_inch(
      screen, &g->stance_icon_interval, &vec2(2, 2));
  locommon_screen_calc_winpos_from_inch(
      screen, &g->stance_iconsz, &vec2(.3f, .3f));
}

static void loui_menu_initialize_stance_(
    loui_menu_t*             menu,
    loui_menu_stance_t*      stance,
    const locommon_screen_t* screen,
    const char*              name,
    const char*              desc,
    const char*              note) {
  assert(menu   != NULL);
  assert(stance != NULL);
  assert(name   != NULL);
  assert(desc   != NULL);
  assert(note   != NULL);
  assert(locommon_screen_valid(screen));

  *stance = (typeof(*stance)) {0};

  const float theta =
      (stance - menu->stances)*1.f/LOPLAYER_STANCE_COUNT*2*MATH_PI +
      MATH_PI/2;
  stance->pos.x = cos(theta) * menu->geo.stance_icon_interval.x;
  stance->pos.y = sin(theta) * menu->geo.stance_icon_interval.y;

  const float  width   = screen->dpi.x*STANCE_TEXT_MAX_WIDTH_;
  const vec2_t invreso = vec2(2/screen->resolution.x, 2/screen->resolution.y);

  stance->name = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
      -menu->geo.big_fontpx.y*STANCE_TEXT_LINEHEIGHT_,
      INT32_MAX,
      STANCE_NAME_RESERVE_);
  glyphas_block_add_characters(
      stance->name,
      menu->font.sans.big,
      &STANCE_NAME_COLOR_,
      name,
      strlen(name));
  glyphas_block_scale(stance->name, &invreso);

  stance->desc = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
      -menu->geo.normal_fontpx.y*STANCE_TEXT_LINEHEIGHT_,
      width,
      STANCE_DESC_RESERVE_);
  glyphas_block_add_characters(
      stance->desc,
      menu->font.sans.normal,
      &STANCE_DESC_COLOR_,
      desc,
      strlen(desc));
  glyphas_block_scale(stance->desc, &invreso);

  stance->note = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
      -menu->geo.small_fontpx.y,
      width,
      STANCE_NOTE_RESERVE_);
  glyphas_block_add_characters(
      stance->note,
      menu->font.serif.small,
      &STANCE_NOTE_COLOR_,
      note,
      strlen(note));
  glyphas_block_scale(stance->note, &invreso);

  glyphas_block_set_origin(stance->name, &vec2(.5f, 0));
  glyphas_block_set_origin(stance->desc, &vec2(.5f, 0));
  glyphas_block_set_origin(stance->note, &vec2(.5f, 0));

  vec2_t dummy;

  vec2_t namesz;
  glyphas_block_calculate_geometry(stance->name, &dummy, &namesz);
  vec2_t descsz;
  glyphas_block_calculate_geometry(stance->desc, &dummy, &descsz);
  vec2_t notesz;
  glyphas_block_calculate_geometry(stance->note, &dummy, &notesz);

  const float note_padding = menu->geo.small_fontsz.y;

  float y = (namesz.y + descsz.y + note_padding + notesz.y)/2;
  glyphas_block_translate(stance->name, &vec2(0, y));

  y -= namesz.y;
  glyphas_block_translate(stance->desc, &vec2(0, y));

  y -= descsz.y + note_padding;
  glyphas_block_translate(stance->note, &vec2(0, y));
}

static void loui_menu_deinitialize_stance_(loui_menu_stance_t* stance) {
  assert(stance != NULL);

  glyphas_block_delete(stance->name);
  glyphas_block_delete(stance->desc);
  glyphas_block_delete(stance->note);
}

static glyphas_block_t* loui_menu_create_exit_text_(
    loui_menu_t* menu, const locommon_screen_t* screen, const char* text) {
  assert(menu   != NULL);
  assert(screen != NULL);
  assert(text   != NULL);

  glyphas_block_t* b = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
      -menu->geo.normal_fontpx.y,
      INT32_MAX,
      32);
  glyphas_block_add_characters(
      b, menu->font.sans.normal, &vec4(1, 1, 1, 1), text, strlen(text));

  glyphas_block_scale(
      b, &vec2(2.f/screen->resolution.x, 2.f/screen->resolution.y));
  glyphas_block_set_origin(b, &vec2(.5f, -1));
  glyphas_block_translate(b, &vec2(0, -1));
  return b;
}

static loui_menu_stance_t* loui_menu_find_stance_icon_at_(
    loui_menu_t* menu, const vec2_t* pos) {
  assert(menu != NULL);
  assert(vec2_valid(pos));

  for (size_t i = 0; i < LOPLAYER_STANCE_COUNT; ++i) {
    vec2_t p;
    vec2_sub(&p, pos, &menu->stances[i].pos);
    p.x /= menu->geo.stance_iconsz.x;
    p.y /= menu->geo.stance_iconsz.y;
    if (fabsf(p.x) + fabsf(p.y) < 1) {
      return &menu->stances[i];
    }
  }
  return NULL;
}

static void loui_menu_update_stances_(
    loui_menu_t* menu, const locommon_input_t* input) {
  assert(menu  != NULL);

  const float dt = menu->ticker->delta_f;

  menu->hovered_stance =
      input == NULL? NULL:
      loui_menu_find_stance_icon_at_(menu, &input->cursor);

  for (size_t i = 0; i < LOPLAYER_STANCE_COUNT; ++i) {
    loui_menu_stance_t* s = &menu->stances[i];
    locommon_easing_linear_float(
        &s->highlight,
        s == menu->hovered_stance,
        dt*STANCE_ICON_HIGHLIGHT_SPEED_);
  }
}

static void loui_menu_draw_stance_icons_(const loui_menu_t* menu) {
  assert(menu != NULL);

  for (size_t i = 0; i < LOPLAYER_STANCE_COUNT; ++i) {
    const loui_menu_stance_t* s = &menu->stances[i];

    const loshader_menu_stance_id_t id =
        loplayer_stance_set_has(&menu->player->stances, i)?
        loplayer_stance_get_menu_shader_id(i):
        LOSHADER_MENU_STANCE_ID_EMPTY;

    vec2_t sz = menu->geo.stance_iconsz;
    vec2_muleq(&sz, 1+powf(s->highlight, 2.f)*STANCE_ICON_HIGHLIGHT_SIZE_);

    loshader_menu_stance_drawer_add_instance(
        &menu->shaders->drawer.menu_stance,
        &(loshader_menu_stance_drawer_instance_t) {
          .id    = id,
          .pos   = s->pos,
          .size  = sz,
          .alpha = menu->alpha,
        });
  }
}

static void loui_menu_draw_stance_text_(const loui_menu_t* menu) {
  assert(menu != NULL);

  if (menu->hovered_stance == NULL) return;

  const size_t index = menu->hovered_stance - menu->stances;

  const loui_menu_stance_t* s = menu->hovered_stance;
  if (!loplayer_stance_set_has(&menu->player->stances, index)) {
    s = &menu->stances[LOPLAYER_STANCE_COUNT];
  }

  loshader_menu_text_drawer_add_block(
      &menu->shaders->drawer.menu_text, s->name);
  loshader_menu_text_drawer_add_block(
      &menu->shaders->drawer.menu_text, s->desc);
  loshader_menu_text_drawer_add_block(
      &menu->shaders->drawer.menu_text, s->note);
}

void loui_menu_initialize(
    loui_menu_t*             menu,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loplayer_t*              player) {
  assert(menu    != NULL);
  assert(res     != NULL);
  assert(shaders != NULL);
  assert(ticker  != NULL);
  assert(player  != NULL);
  assert(locommon_screen_valid(screen));

  *menu = (typeof(*menu)) {
    .shaders = shaders,
    .ticker  = ticker,
    .player  = player,
  };
  loui_menu_calc_geometry_(menu, screen);

  menu->font = (typeof(menu->font)) {
    .sans = {
      .big = glyphas_cache_new(
          shaders->tex.menu_text,
          &res->font.sans,
          menu->geo.big_fontpx.x,
          menu->geo.big_fontpx.y),
      .normal = glyphas_cache_new(
          shaders->tex.menu_text,
          &res->font.sans,
          menu->geo.normal_fontpx.x,
          menu->geo.normal_fontpx.y),
    },
    .serif = {
      .small = glyphas_cache_new(
          shaders->tex.menu_text,
          &res->font.serif,
          menu->geo.small_fontpx.x,
          menu->geo.small_fontpx.y),
    },
  };

# define text_(v) loresource_text_get(res->lang, v)

# define each_(NAME, name)  \
    loui_menu_initialize_stance_(  \
        menu,  \
        &menu->stances[LOPLAYER_STANCE_##NAME],  \
        screen,  \
        text_("stance_"#name"_name"),  \
        text_("stance_"#name"_desc"),  \
        text_("stance_"#name"_note"))
  LOPLAYER_STANCE_EACH(each_);
# undef each_

  loui_menu_initialize_stance_(
      menu, &menu->stances[LOPLAYER_STANCE_COUNT],
      screen,
      text_("stance_unknown_name"),
      text_("stance_unknown_desc"),
      text_("stance_unknown_note"));
  menu->exit_text = loui_menu_create_exit_text_(
      menu, screen, text_("menu_exit"));

# undef text_
}

void loui_menu_deinitialize(loui_menu_t* menu) {
  assert(menu != NULL);

  for (size_t i = 0; i < LOPLAYER_STANCE_COUNT+1; ++i) {
    loui_menu_deinitialize_stance_(&menu->stances[i]);
  }
  glyphas_block_delete(menu->exit_text);
  glyphas_cache_delete(menu->font.sans.big);
  glyphas_cache_delete(menu->font.sans.normal);
  glyphas_cache_delete(menu->font.serif.small);
}

void loui_menu_update(loui_menu_t* menu, const locommon_input_t* input) {
  assert(menu != NULL);

  loui_menu_update_stances_(menu, input);

  menu->request_exit =
      input != NULL &&
      input->buttons & LOCOMMON_INPUT_BUTTON_OK &&
      input->cursor.y < -1+menu->geo.normal_fontsz.y;
}

void loui_menu_draw(const loui_menu_t* menu) {
  assert(menu != NULL);

  if (menu->alpha <= 0) return;

  menu->shaders->drawer.menu_text.alpha       = menu->alpha;
  menu->shaders->drawer.menu_background.alpha = menu->alpha;

  loui_menu_draw_stance_icons_(menu);
  loui_menu_draw_stance_text_(menu);

  loshader_menu_text_drawer_add_block(
      &menu->shaders->drawer.menu_text, menu->exit_text);
}
