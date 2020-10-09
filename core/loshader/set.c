#include "./set.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <GL/glew.h>

#include "util/gleasy/atlas.h"
#include "util/gleasy/framebuffer.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "core/locommon/screen.h"

#include "./backwall.h"
#include "./bullet.h"
#include "./character.h"
#include "./cinescope.h"
#include "./combat_ring.h"
#include "./event_line.h"
#include "./fog.h"
#include "./ground.h"
#include "./hud_bar.h"
#include "./hud_text.h"
#include "./menu_background.h"
#include "./menu_stance.h"
#include "./menu_text.h"
#include "./particle.h"
#include "./pixsort.h"
#include "./popup_text.h"
#include "./posteffect.h"
#include "./uniblock.h"

void loshader_set_initialize(
    loshader_set_t*          set,
    const locommon_screen_t* screen,
    int32_t                  max_msaa) {
  assert(set != NULL);
  assert(locommon_screen_valid(screen));
  assert(max_msaa > 0);

  *set = (typeof(*set)) {0};

  loshader_uniblock_initialize(&set->uniblock, screen);

  int max_samples;
  glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &max_samples);
  gleasy_framebuffer_initialize(
      &set->framebuffer,
      screen->resolution.x,
      screen->resolution.y,
      MATH_MIN(max_samples, max_msaa));

  /* TODO(catfoot): atlas size should depend on DPI. */
  set->tex = (typeof(set->tex)) {
    .hud_text   = gleasy_atlas_new(GL_RED, 1024, 1024, true),
    .popup_text = gleasy_atlas_new(GL_RED, 1024, 1024, true),
    .menu_text  = gleasy_atlas_new(GL_RED, 1024, 1024, true),
    .event_line = gleasy_atlas_new(GL_RED, 1024, 1024, true),
  };

  loshader_backwall_drawer_initialize(&set->drawer.backwall, &set->uniblock);
  loshader_ground_drawer_initialize(&set->drawer.ground, &set->uniblock);
  loshader_character_drawer_initialize(&set->drawer.character, &set->uniblock);
  loshader_bullet_drawer_initialize(&set->drawer.bullet, &set->uniblock);
  loshader_particle_drawer_initialize(&set->drawer.particle, &set->uniblock);
  loshader_fog_drawer_initialize(&set->drawer.fog, &set->uniblock);
  loshader_pixsort_drawer_initialize(
      &set->drawer.pixsort, &set->uniblock, &set->framebuffer);
  loshader_posteffect_drawer_initialize(
      &set->drawer.posteffect, &set->uniblock, &set->framebuffer);
  loshader_hud_bar_drawer_initialize(&set->drawer.hud_bar, &set->uniblock);
  loshader_menu_background_drawer_initialize(
      &set->drawer.menu_background, &set->uniblock);
  loshader_menu_stance_drawer_initialize(
      &set->drawer.menu_stance, &set->uniblock);
  loshader_combat_ring_drawer_initialize(
      &set->drawer.combat_ring, &set->uniblock);
  loshader_cinescope_drawer_initialize(&set->drawer.cinescope, &set->uniblock);

  loshader_hud_text_drawer_initialize(
      &set->drawer.hud_text,
      &set->uniblock,
      gleasy_atlas_get_texture(set->tex.hud_text));

  loshader_popup_text_drawer_initialize(
      &set->drawer.popup_text,
      &set->uniblock,
      gleasy_atlas_get_texture(set->tex.popup_text));

  loshader_menu_text_drawer_initialize(
      &set->drawer.menu_text,
      &set->uniblock,
      gleasy_atlas_get_texture(set->tex.menu_text));

  loshader_event_line_drawer_initialize(
      &set->drawer.event_line,
      &set->uniblock,
      gleasy_atlas_get_texture(set->tex.event_line));
}

void loshader_set_deinitialize(loshader_set_t* set) {
  assert(set != NULL);

  loshader_backwall_drawer_deinitialize(&set->drawer.backwall);
  loshader_ground_drawer_deinitialize(&set->drawer.ground);
  loshader_character_drawer_deinitialize(&set->drawer.character);
  loshader_bullet_drawer_deinitialize(&set->drawer.bullet);
  loshader_particle_drawer_deinitialize(&set->drawer.particle);
  loshader_fog_drawer_deinitialize(&set->drawer.fog);
  loshader_pixsort_drawer_deinitialize(&set->drawer.pixsort);
  loshader_posteffect_drawer_deinitialize(&set->drawer.posteffect);
  loshader_hud_bar_drawer_deinitialize(&set->drawer.hud_bar);
  loshader_hud_text_drawer_deinitialize(&set->drawer.hud_text);
  loshader_popup_text_drawer_deinitialize(&set->drawer.popup_text);
  loshader_menu_background_drawer_deinitialize(&set->drawer.menu_background);
  loshader_menu_text_drawer_deinitialize(&set->drawer.menu_text);
  loshader_menu_stance_drawer_deinitialize(&set->drawer.menu_stance);
  loshader_combat_ring_drawer_deinitialize(&set->drawer.combat_ring);
  loshader_cinescope_drawer_deinitialize(&set->drawer.cinescope);
  loshader_event_line_drawer_deinitialize(&set->drawer.event_line);

  gleasy_atlas_delete(set->tex.hud_text);
  gleasy_atlas_delete(set->tex.popup_text);
  gleasy_atlas_delete(set->tex.menu_text);
  gleasy_atlas_delete(set->tex.event_line);

  loshader_uniblock_deinitialize(&set->uniblock);
  gleasy_framebuffer_deinitialize(&set->framebuffer);
}

void loshader_set_clear_all(loshader_set_t* set) {
  assert(set != NULL);

  set->drawer.hud_text.alpha        = 0;
  set->drawer.menu_background.alpha = 0;
  set->drawer.menu_text.alpha       = 0;

  loshader_ground_drawer_clear(&set->drawer.ground, 256);
  loshader_character_drawer_clear(&set->drawer.character, 256);
  loshader_bullet_drawer_clear(&set->drawer.bullet, 256);
  loshader_particle_drawer_clear(&set->drawer.particle, 256);

  loshader_hud_bar_drawer_clear(&set->drawer.hud_bar, 16);
  loshader_hud_text_drawer_clear(&set->drawer.hud_text, 256);
  loshader_popup_text_drawer_clear(&set->drawer.popup_text, 256);
  loshader_menu_text_drawer_clear(&set->drawer.menu_text, 512);
  loshader_menu_stance_drawer_clear(&set->drawer.menu_stance, 16);
  loshader_combat_ring_drawer_clear(&set->drawer.combat_ring, 8);
  loshader_event_line_drawer_clear(&set->drawer.event_line, 256);
}

void loshader_set_draw_all(const loshader_set_t* set) {
  assert(set != NULL);

  /* ---- path 1: game ---- */
  gleasy_framebuffer_bind(&set->framebuffer);

  glEnable(GL_BLEND);
  glEnable(GL_MULTISAMPLE);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  loshader_backwall_drawer_draw(&set->drawer.backwall);
  loshader_ground_drawer_draw(&set->drawer.ground);
  loshader_character_drawer_draw(&set->drawer.character);
  loshader_bullet_drawer_draw(&set->drawer.bullet);
  loshader_particle_drawer_draw(&set->drawer.particle);
  loshader_fog_drawer_draw(&set->drawer.fog);

  gleasy_framebuffer_flush(&set->framebuffer);

  /* ---- path 2: pixsort ---- */
  if (!loshader_pixsort_drawer_is_skippable(&set->drawer.pixsort)) {
    gleasy_framebuffer_bind(&set->framebuffer);
    loshader_pixsort_drawer_draw(&set->drawer.pixsort);
    gleasy_framebuffer_flush(&set->framebuffer);
  }

  /* ---- path 2: HUD ---- */
  gleasy_framebuffer_bind(&set->framebuffer);

  glDisable(GL_BLEND);
  glDisable(GL_MULTISAMPLE);

  loshader_posteffect_drawer_draw(&set->drawer.posteffect);

  glEnable(GL_BLEND);
  glEnable(GL_MULTISAMPLE);

  loshader_hud_bar_drawer_draw(&set->drawer.hud_bar);
  loshader_hud_text_drawer_draw(&set->drawer.hud_text);
  loshader_popup_text_drawer_draw(&set->drawer.popup_text);
  loshader_menu_background_drawer_draw(&set->drawer.menu_background);
  loshader_menu_text_drawer_draw(&set->drawer.menu_text);
  loshader_menu_stance_drawer_draw(&set->drawer.menu_stance);
  loshader_combat_ring_drawer_draw(&set->drawer.combat_ring);
  loshader_cinescope_drawer_draw(&set->drawer.cinescope);
  loshader_event_line_drawer_draw(&set->drawer.event_line);

  gleasy_framebuffer_flush_to_other(&set->framebuffer, 0);
}

void loshader_set_drop_cache(loshader_set_t* set) {
  assert(set != NULL);

  gleasy_atlas_clear(set->tex.hud_text);
  gleasy_atlas_clear(set->tex.popup_text);
  gleasy_atlas_clear(set->tex.menu_text);
  gleasy_atlas_clear(set->tex.event_line);
}
