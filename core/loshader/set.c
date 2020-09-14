#include "./set.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "util/gleasy/atlas.h"
#include "util/gleasy/framebuffer.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"

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
#include "./posteffect.h"
#include "./uniblock.h"

void loshader_set_initialize(
    loshader_set_t* set,
    int32_t         width,
    int32_t         height,
    const vec2_t*   dpi,
    int32_t         max_msaa) {
  assert(set != NULL);
  assert(width  > 0);
  assert(height > 0);
  assert(vec2_valid(dpi));
  assert(max_msaa > 0);

  *set = (typeof(*set)) {
    .resolution = vec2(width, height),
    .dpi        = *dpi,
  };

  set->uniblock = loshader_uniblock_new();
  loshader_uniblock_update_display_param(set->uniblock, &set->resolution, dpi);

  int max_samples;
  glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &max_samples);
  gleasy_framebuffer_initialize(
      &set->framebuffer, width, height, MATH_MIN(max_samples, max_msaa));

  /* TODO(catfoot): atlas size should depend on DPI. */
  set->tex = (typeof(set->tex)) {
    .hud_text   = gleasy_atlas_new(GL_RED, 1024, 1024, true),
    .menu_text  = gleasy_atlas_new(GL_RED, 1024, 1024, true),
    .event_line = gleasy_atlas_new(GL_RED, 1024, 1024, true),
  };

  loshader_backwall_program_initialize(&set->program.backwall);
  loshader_ground_program_initialize(&set->program.ground);
  loshader_character_program_initialize(&set->program.character);
  loshader_bullet_program_initialize(&set->program.bullet);
  loshader_fog_program_initialize(&set->program.fog);
  loshader_pixsort_program_initialize(&set->program.pixsort);
  loshader_posteffect_program_initialize(&set->program.posteffect);
  loshader_hud_bar_program_initialize(&set->program.hud_bar);
  loshader_hud_text_program_initialize(&set->program.hud_text);
  loshader_menu_background_program_initialize(&set->program.menu_background);
  loshader_menu_text_program_initialize(&set->program.menu_text);
  loshader_menu_stance_program_initialize(&set->program.menu_stance);
  loshader_combat_ring_program_initialize(&set->program.combat_ring);
  loshader_cinescope_program_initialize(&set->program.cinescope);
  loshader_event_line_program_initialize(&set->program.event_line);

  set->drawer = (typeof(set->drawer)) {
    .backwall = loshader_backwall_drawer_new(
        &set->program.backwall, set->uniblock),

    .ground = loshader_ground_drawer_new(
        &set->program.ground, set->uniblock),

    .character = loshader_character_drawer_new(
        &set->program.character, set->uniblock),

    .bullet = loshader_bullet_drawer_new(
        &set->program.bullet, set->uniblock),

    .fog = loshader_fog_drawer_new(
        &set->program.fog, set->uniblock),

    .pixsort = loshader_pixsort_drawer_new(
        &set->program.pixsort, set->uniblock, &set->framebuffer),

    .posteffect = loshader_posteffect_drawer_new(
        &set->program.posteffect, set->uniblock, &set->framebuffer),

    .hud_bar = loshader_hud_bar_drawer_new(
        &set->program.hud_bar, set->uniblock),

    .menu_background = loshader_menu_background_drawer_new(
        &set->program.menu_background, set->uniblock),

    .menu_stance = loshader_menu_stance_drawer_new(
        &set->program.menu_stance, set->uniblock),

    .combat_ring = loshader_combat_ring_drawer_new(
        &set->program.combat_ring, set->uniblock),

    .cinescope = loshader_cinescope_drawer_new(
        &set->program.cinescope, set->uniblock),
  };

  loshader_hud_text_drawer_initialize(
      &set->drawer.hud_text,
      &set->program.hud_text,
      set->uniblock,
      gleasy_atlas_get_texture(set->tex.hud_text));

  loshader_menu_text_drawer_initialize(
      &set->drawer.menu_text,
      &set->program.menu_text,
      set->uniblock,
      gleasy_atlas_get_texture(set->tex.menu_text));

  loshader_event_line_drawer_initialize(
      &set->drawer.event_line,
      &set->program.event_line,
      set->uniblock,
      gleasy_atlas_get_texture(set->tex.event_line));
}

void loshader_set_deinitialize(loshader_set_t* set) {
  assert(set != NULL);

  loshader_backwall_drawer_delete(set->drawer.backwall);
  loshader_ground_drawer_delete(set->drawer.ground);
  loshader_character_drawer_delete(set->drawer.character);
  loshader_bullet_drawer_delete(set->drawer.bullet);
  loshader_fog_drawer_delete(set->drawer.fog);
  loshader_pixsort_drawer_delete(set->drawer.pixsort);
  loshader_posteffect_drawer_delete(set->drawer.posteffect);
  loshader_hud_bar_drawer_delete(set->drawer.hud_bar);
  loshader_hud_text_drawer_deinitialize(&set->drawer.hud_text);
  loshader_menu_background_drawer_delete(set->drawer.menu_background);
  loshader_menu_text_drawer_deinitialize(&set->drawer.menu_text);
  loshader_menu_stance_drawer_delete(set->drawer.menu_stance);
  loshader_combat_ring_drawer_delete(set->drawer.combat_ring);
  loshader_cinescope_drawer_delete(set->drawer.cinescope);
  loshader_event_line_drawer_deinitialize(&set->drawer.event_line);

  loshader_backwall_program_deinitialize(&set->program.backwall);
  loshader_ground_program_deinitialize(&set->program.ground);
  loshader_character_program_deinitialize(&set->program.character);
  loshader_bullet_program_deinitialize(&set->program.bullet);
  loshader_fog_program_deinitialize(&set->program.fog);
  loshader_pixsort_program_deinitialize(&set->program.pixsort);
  loshader_posteffect_program_deinitialize(&set->program.posteffect);
  loshader_hud_bar_program_deinitialize(&set->program.hud_bar);
  loshader_hud_text_program_deinitialize(&set->program.hud_text);
  loshader_menu_background_program_deinitialize(&set->program.menu_background);
  loshader_menu_text_program_deinitialize(&set->program.menu_text);
  loshader_menu_stance_program_deinitialize(&set->program.menu_stance);
  loshader_combat_ring_program_deinitialize(&set->program.combat_ring);
  loshader_cinescope_program_deinitialize(&set->program.cinescope);
  loshader_event_line_program_deinitialize(&set->program.event_line);

  gleasy_atlas_delete(set->tex.hud_text);
  gleasy_atlas_delete(set->tex.menu_text);
  gleasy_atlas_delete(set->tex.event_line);

  loshader_uniblock_delete(set->uniblock);
  gleasy_framebuffer_deinitialize(&set->framebuffer);
}

void loshader_set_clear_all(loshader_set_t* set) {
  assert(set != NULL);

  loshader_ground_drawer_clear(set->drawer.ground, 256);
  loshader_character_drawer_clear(set->drawer.character, 256);
  loshader_bullet_drawer_clear(set->drawer.bullet, 256);

  loshader_hud_bar_drawer_clear(set->drawer.hud_bar, 16);
  loshader_hud_text_drawer_clear(&set->drawer.hud_text, 256);
  loshader_menu_background_drawer_set_alpha(set->drawer.menu_background, 0);
  loshader_menu_text_drawer_clear(&set->drawer.menu_text, 512);
  loshader_menu_stance_drawer_clear(set->drawer.menu_stance, 16);
  loshader_combat_ring_drawer_clear(set->drawer.combat_ring, 8);
  loshader_event_line_drawer_clear(&set->drawer.event_line, 256);
}

void loshader_set_draw_all(const loshader_set_t* set) {
  assert(set != NULL);

  /* ---- path 1: game ---- */
  gleasy_framebuffer_bind(&set->framebuffer);

  glEnable(GL_BLEND);
  glEnable(GL_MULTISAMPLE);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  loshader_backwall_drawer_draw(set->drawer.backwall);
  loshader_ground_drawer_draw(set->drawer.ground);
  loshader_character_drawer_draw(set->drawer.character);
  loshader_bullet_drawer_draw(set->drawer.bullet);
  loshader_fog_drawer_draw(set->drawer.fog);

  gleasy_framebuffer_flush(&set->framebuffer);

  /* ---- path 2: pixsort ---- */
  if (!loshader_pixsort_drawer_is_skippable(set->drawer.pixsort)) {
    gleasy_framebuffer_bind(&set->framebuffer);
    loshader_pixsort_drawer_draw(set->drawer.pixsort);
    gleasy_framebuffer_flush(&set->framebuffer);
  }

  /* ---- path 2: HUD ---- */
  gleasy_framebuffer_bind(&set->framebuffer);

  glDisable(GL_BLEND);
  glDisable(GL_MULTISAMPLE);

  loshader_posteffect_drawer_draw(set->drawer.posteffect);

  glEnable(GL_BLEND);
  glEnable(GL_MULTISAMPLE);

  loshader_hud_bar_drawer_draw(set->drawer.hud_bar);
  loshader_hud_text_drawer_draw(&set->drawer.hud_text);
  loshader_menu_background_drawer_draw(set->drawer.menu_background);
  loshader_menu_text_drawer_draw(&set->drawer.menu_text);
  loshader_menu_stance_drawer_draw(set->drawer.menu_stance);
  loshader_combat_ring_drawer_draw(set->drawer.combat_ring);
  loshader_cinescope_drawer_draw(set->drawer.cinescope);
  loshader_event_line_drawer_draw(&set->drawer.event_line);

  gleasy_framebuffer_flush_to_other(&set->framebuffer, 0);
}

void loshader_set_drop_cache(loshader_set_t* set) {
  assert(set != NULL);

  gleasy_atlas_clear(set->tex.hud_text);
  gleasy_atlas_clear(set->tex.menu_text);
  gleasy_atlas_clear(set->tex.event_line);
}
