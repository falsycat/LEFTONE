#include "./environment.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "util/jukebox/amp.h"
#include "util/jukebox/decoder.h"
#include "util/math/rational.h"

#include "core/locommon/easing.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loresource/music.h"
#include "core/loresource/set.h"
#include "core/loresource/text.h"
#include "core/loshader/set.h"
#include "core/loshader/backwall.h"
#include "core/loshader/fog.h"

#include "./view.h"

#define MUSIC_FADE_DURATION_       rational(1, 1)
#define MUSIC_EVENT_FADE_DURATION_ rational(1, 2)

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
  __builtin_unreachable();
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
  __builtin_unreachable();
}

static loresource_music_id_t loworld_environment_get_music_id_(
    loworld_chunk_biome_t b) {
  switch (b) {
  case LOWORLD_CHUNK_BIOME_METAPHYSICAL_GATE:
    return LORESOURCE_MUSIC_ID_BIOME_METAPHYSICAL_GATE;
  case LOWORLD_CHUNK_BIOME_CAVIAS_CAMP:
    return LORESOURCE_MUSIC_ID_BIOME_CAVIAS_CAMP;
  case LOWORLD_CHUNK_BIOME_LABORATORY:
    return LORESOURCE_MUSIC_ID_BIOME_LABORATORY;
  case LOWORLD_CHUNK_BIOME_BOSS_THEISTS_CHILD:
  case LOWORLD_CHUNK_BIOME_BOSS_BIG_WARDER:
  case LOWORLD_CHUNK_BIOME_BOSS_GREEDY_SCIENTIST:
    return LORESOURCE_MUSIC_ID_BIOME_BOSS;
  }
  __builtin_unreachable();
}

static void loworld_environment_stop_music_(
    loworld_environment_t* env, const rational_t* after) {
  assert(env != NULL);

  if (env->music != NULL) {
    jukebox_amp_change_volume(&env->music->amp, 0, after);
    jukebox_decoder_stop_after(env->music->decoder, after);
  }
  env->music = NULL;
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
  const locommon_position_t* pos  = &env->player->event.ctx.area.pos;
  const vec2_t*              size = &env->player->event.ctx.area.size;
  if (size->x > 0 && size->y > 0) {
    env->fog.bounds_pos  = *pos;
    env->fog.bounds_size = *size;
    locommon_easing_smooth_float(
        &env->fog.bounds_fog, 1, env->ticker->delta_f);
  } else {
    locommon_easing_smooth_float(
        &env->fog.bounds_fog, 0, env->ticker->delta_f);
  }
}

static void loworld_environment_update_music_(loworld_environment_t* env) {
  assert(env != NULL);

  const loplayer_event_context_t* ev = &env->player->event.ctx;

  loresource_music_t* music = env->music;
  if (ev->music.enable) {
    music = loresource_music_set_get(&env->res->music, ev->music.id);
    if (music != env->music) {
      loworld_environment_stop_music_(env, &MUSIC_EVENT_FADE_DURATION_);
      jukebox_amp_change_volume(&music->amp, .9f, &MUSIC_EVENT_FADE_DURATION_);

      const uint64_t t = env->ticker->time - ev->music.since;
      jukebox_decoder_play(music->decoder, &rational(t, 1000), true);
    }
  } else {
    music = loresource_music_set_get(
        &env->res->music, loworld_environment_get_music_id_(env->biome));
    if (music != env->music) {
      loworld_environment_stop_music_(env, &MUSIC_FADE_DURATION_);
      jukebox_amp_change_volume(&music->amp, .6f, &MUSIC_FADE_DURATION_);

      jukebox_decoder_resume(music->decoder, true);
    }
  }
  env->music = music;
}

void loworld_environment_initialize(
    loworld_environment_t*              env,
    loresource_set_t*                   res,
    loshader_set_t*                     shaders,
    const locommon_ticker_t*            ticker,
    const loplayer_t*                   player,
    const loworld_view_t*               view,
    const loworld_environment_config_t* config) {
  assert(env     != NULL);
  assert(res     != NULL);
  assert(shaders != NULL);
  assert(ticker  != NULL);
  assert(player  != NULL);
  assert(view    != NULL);
  assert(config  != NULL);

  *env = (typeof(*env)) {
    .res     = res,
    .shaders = shaders,
    .ticker  = ticker,
    .player  = player,
    .view    = view,

    .config = *config,
  };
}

void loworld_environment_deinitialize(loworld_environment_t* env) {
  assert(env != NULL);

  loworld_environment_stop_music_(env, &rational(1, 10));
}

void loworld_environment_update(loworld_environment_t* env) {
  assert(env != NULL);

  const loworld_chunk_t* chunk = loworld_view_get_looking_chunk(env->view);

  if (env->transition == 1 && env->biome != chunk->biome) {
    env->biome      = chunk->biome;
    env->transition = 0;
  }
  loworld_environment_update_backwall_(env);
  loworld_environment_update_fog_(env);
  loworld_environment_update_music_(env);

  locommon_easing_linear_float(&env->transition, 1, env->ticker->delta_f);
}

void loworld_environment_draw(const loworld_environment_t* env) {
  assert(env != NULL);

  loshader_backwall_drawer_set_param(
      &env->shaders->drawer.backwall, &env->backwall);
  loshader_fog_drawer_set_param(
      &env->shaders->drawer.fog, &env->fog);
}
