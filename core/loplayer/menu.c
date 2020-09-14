#include "./menu.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "util/glyphas/block.h"
#include "util/glyphas/cache.h"
#include "util/math/algorithm.h"
#include "util/math/constant.h"
#include "util/memory/memory.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/easing.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/stance.h"
#include "core/loresource/set.h"
#include "core/loresource/text.h"
#include "core/loshader/set.h"

#include "./controller.h"
#include "./entity.h"
#include "./status.h"

typedef struct {
  glyphas_block_t* name;
  glyphas_block_t* desc;
  glyphas_block_t* note;
} loplayer_menu_stance_text_set_t;

typedef enum {
  LOPLAYER_MENU_STATE_HIDDEN,
  LOPLAYER_MENU_STATE_STATUS,
  LOPLAYER_MENU_STATE_POPUP,
} loplayer_menu_state_t;

struct loplayer_menu_t {
  /* injected deps */
  loresource_set_t* res;
  loshader_set_t*   shaders;

  const locommon_ticker_t*     ticker;
  const loplayer_status_t*     status;
  const loplayer_controller_t* controller;

  /* owned objects */
  struct {
    glyphas_cache_t* large;
    glyphas_cache_t* normal;
    glyphas_cache_t* small;
  } font;

  struct {
    loplayer_menu_stance_text_set_t stance[LOEFFECT_STANCE_ID_LENGTH_+1];
        /* [0] is for unknown stance */

    glyphas_block_t* popup_title;
    glyphas_block_t* popup_body;

    glyphas_block_t* exit;
  } text;

  /* immutable params */
  struct {
    vec2_t fontsz_large;
    vec2_t fontsz_normal;
    vec2_t fontsz_small;

    vec2_t fontsz_large_px;
    vec2_t fontsz_normal_px;
    vec2_t fontsz_small_px;

    vec2_t range;
    vec2_t icon;
    vec2_t padding;
  } geometry;

  /* mutable params */
  loplayer_menu_state_t state;
  float                 alpha;

  bool   stance_hovering;
  size_t display_stance_text_index;

  float  stance_highlight;
  size_t highlighted_stance;

  bool request_exit;
};

static void loplayer_menu_calculate_geometry_(loplayer_menu_t* menu) {
  assert(menu != NULL);

  const vec2_t* dpi  = &menu->shaders->dpi;
  const vec2_t* reso = &menu->shaders->resolution;

  typeof(menu->geometry)* geo = &menu->geometry;

  geo->fontsz_large_px = *dpi;
  vec2_muleq(&geo->fontsz_large_px, .2f);

  geo->fontsz_normal_px = *dpi;
  vec2_muleq(&geo->fontsz_normal_px, .15f);

  geo->fontsz_small_px = *dpi;
  vec2_muleq(&geo->fontsz_small_px, .12f);

# define px_to_disp_(v) vec2((v).x/reso->x*2, (v).y/reso->y*2)

  geo->fontsz_large  = px_to_disp_(geo->fontsz_large_px);
  geo->fontsz_normal = px_to_disp_(geo->fontsz_normal_px);
  geo->fontsz_small  = px_to_disp_(geo->fontsz_small_px);

  geo->range = *dpi;
  vec2_muleq(&geo->range, 2);
  geo->range = px_to_disp_(geo->range);
  vec2_diveq(&geo->range, MATH_MAX(geo->range.x, 1));
  vec2_diveq(&geo->range, MATH_MAX(geo->range.y, 1));

  geo->icon = *dpi;
  vec2_muleq(&geo->icon, .4f);
  geo->icon = px_to_disp_(geo->icon);

  geo->padding = *dpi;
  vec2_muleq(&geo->padding, .2f);
  geo->padding = px_to_disp_(geo->padding);

# undef px_to_disp_
}

static void loplayer_menu_initialize_stance_text_set_(
    const loplayer_menu_t*           menu,
    loplayer_menu_stance_text_set_t* set,
    const char*                      name,
    const char*                      desc,
    const char*                      note) {
  assert(menu != NULL);
  assert(set  != NULL);

  const typeof(menu->geometry)* geo = &menu->geometry;

  *set = (typeof(*set)) {
    .name = glyphas_block_new(
        GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
        -geo->fontsz_large_px.y,
        INT32_MAX,
        32),
    .desc = glyphas_block_new(
        GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
        -geo->fontsz_normal_px.y*1.3f,
        geo->fontsz_normal_px.x*18,
        512),
    .note = glyphas_block_new(
        GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
        -geo->fontsz_small_px.y*1.1f,
        geo->fontsz_normal_px.x*18,
        512),
  };

  static const vec4_t white  = vec4(1, 1, 1, 1);
  static const vec4_t yellow = vec4(1, .7f, 0, 1);

  glyphas_block_add_characters(
      set->name, menu->font.large, &white, name, strlen(name));
  glyphas_block_add_characters(
      set->desc, menu->font.normal, &white, desc, strlen(desc));
  glyphas_block_add_characters(
      set->note, menu->font.small, &yellow, note, strlen(note));

  glyphas_block_set_origin(set->name, &vec2(.5f, -1));
  glyphas_block_set_origin(set->desc, &vec2(.5f, -.5f));
  glyphas_block_set_origin(set->note, &vec2(.5f, 0));

  const vec2_t scale =
      vec2(2/menu->shaders->resolution.x, 2/menu->shaders->resolution.y);
  glyphas_block_scale(set->name, &scale);
  glyphas_block_scale(set->desc, &scale);
  glyphas_block_scale(set->note, &scale);

  vec2_t offset, size;
  glyphas_block_calculate_geometry(set->desc, &offset, &size);

  glyphas_block_translate(set->name, &vec2(0, size.y/2 * 1.1f));
  glyphas_block_translate(set->note, &vec2(0, -size.y/2 * 1.2f));
}

static void loplayer_menu_deinitialize_stance_text_set_(
    loplayer_menu_stance_text_set_t* set) {
  assert(set != NULL);

  glyphas_block_delete(set->name);
  glyphas_block_delete(set->desc);
  glyphas_block_delete(set->note);
}

static void loplayer_menu_create_stance_text_(loplayer_menu_t* menu) {
  assert(menu != NULL);

# define text_(name) loresource_text_get(menu->res->lang, name)

  loplayer_menu_initialize_stance_text_set_(
      menu, &menu->text.stance[0], "???", "???", "");

# define init_stance_set_(NAME, name)  \
    loplayer_menu_initialize_stance_text_set_(  \
        menu,  \
        &menu->text.stance[LOEFFECT_STANCE_ID_##NAME+1],  \
        text_("stance_"#name"_name"),  \
        text_("stance_"#name"_desc"),  \
        text_("stance_"#name"_note"))

  LOEFFECT_STANCE_EACH(init_stance_set_);

# undef init_stance_set_
# undef text_
}

static void loplayer_menu_create_exit_text_(loplayer_menu_t* menu) {
  assert(menu != NULL);

  menu->text.exit = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
      -menu->geometry.fontsz_normal_px.y,
      INT32_MAX,
      32);

  const char* text = loresource_text_get(menu->res->lang, "menu_exit");

  static const vec4_t white = vec4(1, 1, 1, 1);
  glyphas_block_add_characters(
      menu->text.exit, menu->font.normal, &white, text, strlen(text));

  glyphas_block_set_origin(menu->text.exit, &vec2(.5f, -1));

  const vec2_t scale =
      vec2(2/menu->shaders->resolution.x, 2/menu->shaders->resolution.y);
  glyphas_block_scale(menu->text.exit, &scale);

  glyphas_block_translate(menu->text.exit, &vec2(0, -1+menu->geometry.padding.y));
}

static void loplayer_menu_build_popup_text_(
    loplayer_menu_t* menu, const char* title, const char* text) {
  assert(menu != NULL);

  glyphas_block_clear(menu->text.popup_title);
  glyphas_block_clear(menu->text.popup_body);

  static const vec4_t white = vec4(1, 1, 1, 1);
  glyphas_block_add_characters(
      menu->text.popup_title, menu->font.large, &white, title, strlen(title));
  glyphas_block_add_characters(
      menu->text.popup_body, menu->font.normal, &white, text, strlen(text));

  static const vec2_t bottom = vec2(.5f, -1);
  static const vec2_t center = vec2(.5f, -.5f);
  glyphas_block_set_origin(menu->text.popup_title, &bottom);
  glyphas_block_set_origin(menu->text.popup_body,  &center);

  const vec2_t scale =
      vec2(2/menu->shaders->resolution.x, 2/menu->shaders->resolution.y);
  glyphas_block_scale(menu->text.popup_title, &scale);
  glyphas_block_scale(menu->text.popup_body,  &scale);

  vec2_t offset, size;
  glyphas_block_calculate_geometry(menu->text.popup_body, &offset, &size);

  const vec2_t upper = vec2(0, size.y/2*1.1f);
  glyphas_block_translate(menu->text.popup_title, &upper);
}

static bool loplayer_menu_find_stance_icon_at_(
    const loplayer_menu_t* menu, size_t* index, const vec2_t* pos) {
  assert(menu  != NULL);
  assert(index != NULL);
  assert(vec2_valid(pos));

  const typeof(menu->geometry)* geo = &menu->geometry;

  for (size_t i = 0; i < 5; ++i) {
    const float  theta = MATH_PI*2/5 * i + MATH_PI/2;
    const vec2_t p     =
        vec2(geo->range.x*cos(theta), geo->range.y*sin(theta));

    vec2_t diff = *pos;
    vec2_subeq(&diff, &p);

    diff.x /= geo->icon.x;
    diff.y /= geo->icon.y;

    const float manhattan = MATH_ABS(diff.x) + MATH_ABS(diff.y);
    if (manhattan < 1) {
      *index = i;
      return true;
    }
  }
  return false;
}

static void loplayer_menu_update_status_(loplayer_menu_t* menu) {
  assert(menu != NULL);

  if (menu->alpha != 1) return;

  locommon_easing_linear_float(
      &menu->stance_highlight, 0, menu->ticker->delta_f);

  size_t hovered;
  menu->stance_hovering = loplayer_menu_find_stance_icon_at_(
      menu, &hovered, &menu->controller->cursor);
  if (menu->stance_hovering) {
    const bool taken =
        loeffect_stance_set_has(&menu->status->stances, hovered);
    menu->display_stance_text_index = taken? hovered+1: 0;
  }

  const typeof(menu->geometry)* geo = &menu->geometry;
  if (menu->controller->cursor.y < -1+geo->fontsz_normal.y+geo->padding.y) {
    if (menu->controller->prev.buttons & LOCOMMON_INPUT_BUTTON_ATTACK) {
      menu->request_exit = true;
    }
  }
}

static void loplayer_menu_update_popup_(loplayer_menu_t* menu) {
  assert(menu != NULL);

}

static void loplayer_menu_draw_stance_icons_(const loplayer_menu_t* menu) {
  assert(menu != NULL);

  const typeof(menu->geometry)* geo = &menu->geometry;

  for (size_t i = 0; i < 5; ++i) {
    const bool taken =
        loeffect_stance_set_has(&menu->status->stances, i);

    const float theta = MATH_PI*2/5 * i + MATH_PI/2;

    const loshader_menu_stance_drawer_instance_t instance = {
      .id = taken?
          loeffect_stance_get_id_for_menu_shader(i):
          LOSHADER_MENU_STANCE_ID_EMPTY,
      .pos       = vec2(geo->range.x*cos(theta), geo->range.y*sin(theta)),
      .size      = geo->icon,
      .alpha     = menu->alpha,
      .highlight = menu->stance_highlight * !!(menu->highlighted_stance == i),
    };
    loshader_menu_stance_drawer_add_instance(
        menu->shaders->drawer.menu_stance, &instance);
  }
}

static void loplayer_menu_draw_status_(const loplayer_menu_t* menu) {
  assert(menu != NULL);

  loplayer_menu_draw_stance_icons_(menu);

  if (menu->stance_hovering) {
    const loplayer_menu_stance_text_set_t* set =
        &menu->text.stance[menu->display_stance_text_index];
    loshader_menu_text_drawer_add_block(
        &menu->shaders->drawer.menu_text, set->name);
    loshader_menu_text_drawer_add_block(
        &menu->shaders->drawer.menu_text, set->desc);
    loshader_menu_text_drawer_add_block(
        &menu->shaders->drawer.menu_text, set->note);
  }
  loshader_menu_text_drawer_add_block(
      &menu->shaders->drawer.menu_text, menu->text.exit);
}

static void loplayer_menu_draw_popup_(const loplayer_menu_t* menu) {
  assert(menu != NULL);

  loshader_menu_text_drawer_add_block(
      &menu->shaders->drawer.menu_text, menu->text.popup_title);
  loshader_menu_text_drawer_add_block(
      &menu->shaders->drawer.menu_text, menu->text.popup_body);
}

loplayer_menu_t* loplayer_menu_new(
    loresource_set_t*            res,
    loshader_set_t*              shaders,
    const locommon_ticker_t*     ticker,
    const loplayer_status_t*     status,
    const loplayer_controller_t* controller) {
  assert(res        != NULL);
  assert(shaders    != NULL);
  assert(ticker     != NULL);
  assert(status     != NULL);
  assert(controller != NULL);

  loplayer_menu_t* menu = memory_new(sizeof(*menu));
  *menu = (typeof(*menu)) {
    .res        = res,
    .shaders    = shaders,
    .ticker     = ticker,
    .status     = status,
    .controller = controller,
  };

  loplayer_menu_calculate_geometry_(menu);

  menu->font = (typeof(menu->font)) {
    .large = glyphas_cache_new(
        shaders->tex.menu_text,
        &res->font.sans,
        menu->geometry.fontsz_large_px.x,
        menu->geometry.fontsz_large_px.y),
    .normal = glyphas_cache_new(
        shaders->tex.menu_text,
        &res->font.sans,
        menu->geometry.fontsz_normal_px.x,
        menu->geometry.fontsz_normal_px.y),
    .small = glyphas_cache_new(
        shaders->tex.menu_text,
        &res->font.serif,
        menu->geometry.fontsz_small_px.x,
        menu->geometry.fontsz_small_px.y),
  };

  menu->text = (typeof(menu->text)) {
    .popup_title = glyphas_block_new(
        GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
        -menu->geometry.fontsz_large_px.y,
        INT32_MAX,
        32),
    .popup_body = glyphas_block_new(
        GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL,
        -menu->geometry.fontsz_normal_px.y*1.3f,
        menu->geometry.fontsz_normal_px.x*18,
        512),
  };
  loplayer_menu_create_stance_text_(menu);
  loplayer_menu_create_exit_text_(menu);

  return menu;
}

void loplayer_menu_delete(loplayer_menu_t* menu) {
  if (menu == NULL) return;

  static const size_t len =
      sizeof(menu->text.stance)/sizeof(menu->text.stance[0]);
  for (size_t i = 0; i < len; ++i) {
    loplayer_menu_deinitialize_stance_text_set_(&menu->text.stance[i]);
  }

  glyphas_block_delete(menu->text.popup_title);
  glyphas_block_delete(menu->text.popup_body);
  glyphas_block_delete(menu->text.exit);

  glyphas_cache_delete(menu->font.large);
  glyphas_cache_delete(menu->font.normal);
  glyphas_cache_delete(menu->font.small);

  memory_delete(menu);
}

void loplayer_menu_show_status(loplayer_menu_t* menu) {
  assert(menu != NULL);

  menu->state            = LOPLAYER_MENU_STATE_STATUS;
  menu->stance_hovering  = false;
  menu->stance_highlight = 0;
}

void loplayer_menu_show_status_with_stance_highlighted(
    loplayer_menu_t* menu, loeffect_stance_id_t id) {
  assert(menu != NULL);

  loplayer_menu_show_status(menu);
  menu->stance_highlight   = 1;
  menu->highlighted_stance = id;
}

void loplayer_menu_popup(
    loplayer_menu_t* menu, const char* title, const char* text) {
  assert(menu != NULL);

  menu->state = LOPLAYER_MENU_STATE_POPUP;
  loplayer_menu_build_popup_text_(menu, title, text);
}

void loplayer_menu_hide(loplayer_menu_t* menu) {
  assert(menu != NULL);

  menu->state = LOPLAYER_MENU_STATE_HIDDEN;
}

void loplayer_menu_update(loplayer_menu_t* menu) {
  assert(menu != NULL);

  if (menu->state == LOPLAYER_MENU_STATE_HIDDEN) {
    locommon_easing_linear_float(&menu->alpha, 0, menu->ticker->delta_f*2);
    return;
  }
  locommon_easing_linear_float(&menu->alpha, 1, menu->ticker->delta_f*2);

  switch (menu->state) {
  case LOPLAYER_MENU_STATE_STATUS:
    loplayer_menu_update_status_(menu);
    return;
  case LOPLAYER_MENU_STATE_POPUP:
    loplayer_menu_update_popup_(menu);
    return;
  default:
    assert(false);
  }
}

void loplayer_menu_draw_ui(const loplayer_menu_t* menu) {
  assert(menu != NULL);

  loshader_menu_background_drawer_set_alpha(
      menu->shaders->drawer.menu_background, menu->alpha);
  menu->shaders->drawer.menu_text.alpha = menu->alpha;

  switch (menu->state) {
  case LOPLAYER_MENU_STATE_HIDDEN:
    return;
  case LOPLAYER_MENU_STATE_STATUS:
    loplayer_menu_draw_status_(menu);
    return;
  case LOPLAYER_MENU_STATE_POPUP:
    loplayer_menu_draw_popup_(menu);
    return;
  default:
    assert(false);
  }
}

bool loplayer_menu_is_shown(const loplayer_menu_t* menu) {
  assert(menu != NULL);

  return menu->state != LOPLAYER_MENU_STATE_HIDDEN;
}

bool loplayer_menu_is_exit_requested(const loplayer_menu_t* menu) {
  assert(menu != NULL);

  return menu->request_exit;
}

void loplayer_menu_pack(const loplayer_menu_t* menu, msgpack_packer* packer) {
  assert(menu   != NULL);
  assert(packer != NULL);

  mpkutil_pack_bool(packer, menu->state != LOPLAYER_MENU_STATE_HIDDEN);
}

bool loplayer_menu_unpack(loplayer_menu_t* menu, const msgpack_object* obj) {
  assert(menu != NULL);

  bool shown = false;
  if (!mpkutil_get_bool(obj, &shown)) return false;

  if (shown) loplayer_menu_show_status(menu);
  return true;
}
