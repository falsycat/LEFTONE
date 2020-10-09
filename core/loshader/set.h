#pragma once

#include <stdint.h>

#include "util/gleasy/atlas.h"
#include "util/gleasy/framebuffer.h"
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

typedef struct {
  gleasy_framebuffer_t framebuffer;
  loshader_uniblock_t  uniblock;

  struct {
    gleasy_atlas_t* hud_text;
    gleasy_atlas_t* popup_text;
    gleasy_atlas_t* menu_text;
    gleasy_atlas_t* event_line;
  } tex;

  struct {
    loshader_backwall_drawer_t        backwall;
    loshader_ground_drawer_t          ground;
    loshader_character_drawer_t       character;
    loshader_bullet_drawer_t          bullet;
    loshader_particle_drawer_t        particle;
    loshader_fog_drawer_t             fog;
    loshader_pixsort_drawer_t         pixsort;
    loshader_posteffect_drawer_t      posteffect;
    loshader_hud_bar_drawer_t         hud_bar;
    loshader_hud_text_drawer_t        hud_text;
    loshader_popup_text_drawer_t      popup_text;
    loshader_menu_background_drawer_t menu_background;
    loshader_menu_text_drawer_t       menu_text;
    loshader_menu_stance_drawer_t     menu_stance;
    loshader_combat_ring_drawer_t     combat_ring;
    loshader_cinescope_drawer_t       cinescope;
    loshader_event_line_drawer_t      event_line;
  } drawer;
} loshader_set_t;

void
loshader_set_initialize(
    loshader_set_t*          set,
    const locommon_screen_t* screen,
    int32_t                  max_msaa
);

void
loshader_set_deinitialize(
    loshader_set_t* set
);

void
loshader_set_clear_all(
    loshader_set_t* set
);

void
loshader_set_draw_all(
    const loshader_set_t* set
);

void
loshader_set_drop_cache(
    loshader_set_t* set
);
