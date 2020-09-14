#include "./environment.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "util/jukebox/amp.h"
#include "util/jukebox/decoder.h"
#include "util/math/rational.h"

#include "core/locommon/easing.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/event.h"
#include "core/loplayer/player.h"
#include "core/loresource/music.h"
#include "core/loresource/set.h"
#include "core/loresource/text.h"
#include "core/loshader/set.h"
#include "core/loshader/backwall.h"
#include "core/loshader/fog.h"

#include "./view.h"

static const char* loworld_environment_get_chunk_name_(
    loresource_language_t lang, loworld_chunk_biome_t b) {
  switch (b) {
  case LOWORLD_CHUNK_BIOME_METAPHYSICAL_GATE:
    return loresource_text_get(lang, "biome_metaphysical_gate");
  case LOWORLD_CHUNK_BIOME_CAVIAS_CAMP:
    return loresource_text_get(lang, "biome_cavias_camp");
  case LOWORLD_CHUNK_BIOME_LABORATORY:
    return loresource_text_get(lang, "biome_laboratory");
  case LOWORLD_CHUNK_BIOME_BOSS_THEISTS_CHILD:
    return loresource_text_get(lang, "biome_boss_theists_child");
  case LOWORLD_CHUNK_BIOME_BOSS_BIG_WARDER:
    return loresource_text_get(lang, "biome_boss_big_warder");
  case LOWORLD_CHUNK_BIOME_BOSS_GREEDY_SCIENTIST:
    return loresource_text_get(lang, "biome_boss_greedy_scientist");
  }
  assert(false);
  return "unknown biome";
}

static loshader_backwall_type_t loworld_environment_get_backwall_type_(
    loworld_chunk_biome_t b) {
  switch (b) {
  case LOWORLD_CHUNK_BIOME_METAPHYSICAL_GATE:
    return LOSHADER_BACKWALL_TYPE_INFINITE_BOXES;
  case LOWORLD_CHUNK_BIOME_CAVIAS_CAMP:
    return LOSHADER_BACKWALL_TYPE_HOLLOW_MOUNTAINS;
  case LOWORLD_CHUNK_BIOME_LABORATORY:
    return LOSHADER_BACKWALL_TYPE_FABRIC;
  case LOWORLD_CHUNK_BIOME_BOSS_THEISTS_CHILD:
    return LOSHADER_BACKWALL_TYPE_HOLLOW_MOUNTAINS_RED;
  case LOWORLD_CHUNK_BIOME_BOSS_BIG_WARDER:
    return LOSHADER_BACKWALL_TYPE_JAIL;
  case LOWORLD_CHUNK_BIOME_BOSS_GREEDY_SCIENTIST:
    return LOSHADER_BACKWALL_TYPE_INFINITE_BOXES;
  }
  assert(false);
  return LOSHADER_BACKWALL_TYPE_WHITE;
}

static loshader_fog_type_t loworld_environment_get_fog_type_(
    loworld_chunk_biome_t b) {
  switch (b) {
  case LOWORLD_CHUNK_BIOME_METAPHYSICAL_GATE:
  case LOWORLD_CHUNK_BIOME_CAVIAS_CAMP:
  case LOWORLD_CHUNK_BIOME_LABORATORY:
  case LOWORLD_CHUNK_BIOME_BOSS_THEISTS_CHILD:
  case LOWORLD_CHUNK_BIOME_BOSS_BIG_WARDER:
  case LOWORLD_CHUNK_BIOME_BOSS_GREEDY_SCIENTIST:
    return LOSHADER_FOG_TYPE_WHITE_CLOUD;
  }
  assert(false);
  return LOSHADER_FOG_TYPE_NONE;
}

static loresource_music_player_t* loworld_environment_get_music_(
    loworld_chunk_biome_t b, loresource_music_t* m) {
  switch (b) {
  case LOWORLD_CHUNK_BIOME_METAPHYSICAL_GATE:
    return &m->biome_metaphysical_gate;
  case LOWORLD_CHUNK_BIOME_CAVIAS_CAMP:
    return &m->biome_cavias_camp;
  case LOWORLD_CHUNK_BIOME_LABORATORY:
    return &m->biome_laboratory;
  case LOWORLD_CHUNK_BIOME_BOSS_THEISTS_CHILD:
  case LOWORLD_CHUNK_BIOME_BOSS_BIG_WARDER:
  case LOWORLD_CHUNK_BIOME_BOSS_GREEDY_SCIENTIST:
    return &m->biome_boss;
  }
  assert(false);
  return NULL;
}

static void loworld_environment_stop_music_(loworld_environment_t* env) {
  assert(env != NULL);
  if (env->music != NULL && env->music_control) {
    jukebox_amp_change_volume(&env->music->amp, 0, &rational(1, 1));
    jukebox_decoder_stop_after(env->music->decoder, &rational(1, 1));
  }
  env->music = NULL;
}

static void loworld_environment_update_hud_(loworld_environment_t* env) {
  assert(env != NULL);

  if (env->transition > 0) return;

  loplayer_hud_set_biome_text(
      env->player->hud,
      loworld_environment_get_chunk_name_(env->res->lang, env->biome));
}

static void loworld_environment_update_backwall_(loworld_environment_t* env) {
  assert(env != NULL);

  loshader_backwall_type_t prev = env->backwall.prev_type;

  if (env->transition == 0) prev = env->backwall.type;

  env->backwall = (loshader_backwall_drawer_param_t) {
    .prev_type  = prev,
    .type       = LOSHADER_BACKWALL_TYPE_WHITE,
    .transition = env->transition,
  };
  if (!env->config.disable_heavy_backwall) {
    env->backwall.type = loworld_environment_get_backwall_type_(env->biome);
  }
}

static void loworld_environment_update_fog_(loworld_environment_t* env) {
  assert(env != NULL);

  if (env->transition == 0) env->fog.prev_type = env->fog.type;

  env->fog.type = LOSHADER_FOG_TYPE_NONE;
  if (!env->config.disable_heavy_fog) {
    env->fog.type = loworld_environment_get_fog_type_(env->biome);
  }
  env->fog.transition = env->transition;

  /* ---- bounds fog ---- */
  const loplayer_event_param_t* e =
      loplayer_event_get_param(env->player->event);
  if (e != NULL && vec2_pow_length(&e->area_size) > 0) {
    env->fog.bounds_pos  = e->area_pos;
    env->fog.bounds_size = e->area_size;

    locommon_easing_smooth_float(
        &env->fog.bounds_fog, 1, env->ticker->delta_f);
  } else {
    locommon_easing_smooth_float(
        &env->fog.bounds_fog, 0, env->ticker->delta_f);
  }
}

static void loworld_environment_update_music_(loworld_environment_t* env) {
  assert(env != NULL);

  bool control = true;
  loresource_music_player_t* music =
      loworld_environment_get_music_(env->biome, &env->res->music);

  const loplayer_event_param_t* e =
      loplayer_event_get_param(env->player->event);
  if (e != NULL) {
    music   = e->music;
    control = false;

    if (!env->sound_attenuation) {
      loresource_sound_change_master_volume(
          env->res->sound, .2f, &rational(1, 1));
      env->sound_attenuation = true;
    }

  } else {
    if (env->sound_attenuation) {
      loresource_sound_change_master_volume(
          env->res->sound, 1, &rational(1, 1));
      env->sound_attenuation = false;
    }
  }

  if (music != env->music) {
    loworld_environment_stop_music_(env);
    env->music         = music;
    env->music_control = control;
    if (env->music != NULL && env->music_control) {
      jukebox_amp_change_volume(&env->music->amp, .6f, &rational(1, 1));
      jukebox_decoder_resume(env->music->decoder, true);
    }
  }
}

void loworld_environment_initialize(
    loworld_environment_t*              env,
    loresource_set_t*                   res,
    loshader_set_t*                     shaders,
    const locommon_ticker_t*            ticker,
    const loworld_view_t*               view,
    loplayer_t*                         player,
    const loworld_environment_config_t* config) {
  assert(env     != NULL);
  assert(res     != NULL);
  assert(shaders != NULL);
  assert(ticker  != NULL);
  assert(view    != NULL);
  assert(player  != NULL);
  assert(config  != NULL);

  *env = (typeof(*env)) {
    .res     = res,
    .shaders = shaders,
    .ticker  = ticker,
    .view    = view,
    .player  = player,

    .config = *config,
  };
}

void loworld_environment_deinitialize(loworld_environment_t* env) {
  assert(env != NULL);

  loworld_environment_stop_music_(env);
}

void loworld_environment_update(loworld_environment_t* env) {
  assert(env != NULL);

  const loworld_chunk_t* chunk = loworld_view_get_looking_chunk(env->view);

  if (env->transition == 1 && env->biome != chunk->biome) {
    env->biome      = chunk->biome;
    env->transition = 0;
  }
  loworld_environment_update_hud_(env);
  loworld_environment_update_backwall_(env);
  loworld_environment_update_fog_(env);
  loworld_environment_update_music_(env);

  locommon_easing_linear_float(&env->transition, 1, env->ticker->delta_f);
}

void loworld_environment_draw(const loworld_environment_t* env) {
  assert(env != NULL);

  loshader_backwall_drawer_set_param(
      env->shaders->drawer.backwall, &env->backwall);
  loshader_fog_drawer_set_param(
      env->shaders->drawer.fog, &env->fog);
}
