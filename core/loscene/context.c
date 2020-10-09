#include "./context.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/flasy/flasy.h"
#include "util/glyphas/context.h"
#include "util/jukebox/mixer.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "core/locommon/input.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./game.h"
#include "./param.h"
#include "./scene.h"
#include "./title.h"

#define MIXER_RESERVE_ 256

#define FLASY_BUFSIZE_  (1024*4)  /* = 4 KB */
#define FLASY_HANDLERS_ 256

static const jukebox_format_t loscene_context_mixer_format_ = {
  .sample_rate = 48000,
  .channels    = 2,
};

static loscene_t* loscene_context_create_start_scene_(loscene_context_t* ctx) {
  assert(ctx != NULL);

  if (ctx->param.skip_title) {
    return loscene_game_new(ctx, true  /* = load */);
  }
  return loscene_title_new(ctx);
}

void loscene_context_initialize(
    loscene_context_t* ctx, const loscene_param_t* param) {
  assert(ctx   != NULL);
  assert(param != NULL);

  *ctx = (typeof(*ctx)) {
    .param = *param,
  };

  ctx->flasy = flasy_new(FLASY_BUFSIZE_, FLASY_HANDLERS_);

  glyphas_context_initialize(&ctx->glyphas);

  ctx->mixer = jukebox_mixer_new(
      &loscene_context_mixer_format_, MIXER_RESERVE_);

  ctx->screen = (typeof(ctx->screen)) {
    .resolution = vec2(param->width, param->height),
    .dpi        = param->dpi,
  };
  locommon_ticker_initialize(&ctx->ticker, 0);

  loresource_set_initialize(
      &ctx->resources,
      ctx->mixer,
      &loscene_context_mixer_format_,
      LORESOURCE_LANGUAGE_JP);

  loshader_set_initialize(
      &ctx->shaders,
      &ctx->screen,
      param->max_msaa);

  ctx->scene = loscene_context_create_start_scene_(ctx);
}

void loscene_context_deinitialize(loscene_context_t* ctx) {
  assert(ctx != NULL);

  /* Firstly delete the mixer working in other thread. */
  jukebox_mixer_delete(ctx->mixer);

  loscene_delete(ctx->scene);

  loshader_set_deinitialize(&ctx->shaders);
  loresource_set_deinitialize(&ctx->resources);

  locommon_ticker_deinitialize(&ctx->ticker);

  glyphas_context_deinitialize(&ctx->glyphas);

  flasy_delete(ctx->flasy);
}

bool loscene_context_update(
    loscene_context_t* ctx, const locommon_input_t* input, uint64_t uptime) {
  assert(ctx   != NULL);
  assert(input != NULL);

  locommon_ticker_tick(&ctx->ticker, uptime);

  loscene_t* s = loscene_update(ctx->scene, input);
  if (s != ctx->scene) {
    loscene_delete(ctx->scene);
    ctx->scene = s;
  }
  return ctx->scene != NULL;
}

void loscene_context_draw(loscene_context_t* ctx) {
  assert(ctx != NULL);

  loscene_draw(ctx->scene);
}
