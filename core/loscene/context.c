#include "./context.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/glyphas/context.h"
#include "util/jukebox/mixer.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "core/locommon/input.h"
#include "core/locommon/ticker.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./game.h"
#include "./param.h"
#include "./scene.h"
#include "./title.h"

struct loscene_context_t {
  glyphas_context_t glyphas;
  jukebox_mixer_t*  mixer;

  locommon_ticker_t ticker;

  loresource_set_t resources;
  loshader_set_t   shaders;

  loscene_t* scene;

  loscene_param_t param;
};

#define loscene_context_mixer_reserve_ 256

static const jukebox_format_t loscene_context_mixer_format_ = {
  .sample_rate = 48000,
  .channels    = 2,
};

static loscene_t* loscene_context_create_start_scene_(loscene_context_t* ctx) {
  assert(ctx != NULL);

  /* Unless the context is deleted, scenes can hold a pointer to ctx->param. */

  if (ctx->param.skip_title) {
    return loscene_game_new(
        &ctx->param, &ctx->resources, &ctx->shaders, &ctx->ticker, true);
  }
  return loscene_title_new(
      &ctx->param, &ctx->resources, &ctx->shaders, &ctx->ticker);
}

loscene_context_t* loscene_context_new(const loscene_param_t* param) {
  assert(param != NULL);

  loscene_context_t* ctx = memory_new(sizeof(*ctx));
  *ctx = (typeof(*ctx)) {
    .param = *param,
  };

  glyphas_context_initialize(&ctx->glyphas);

  ctx->mixer = jukebox_mixer_new(
      &loscene_context_mixer_format_, loscene_context_mixer_reserve_);

  locommon_ticker_initialize(&ctx->ticker, 0);

  loresource_set_initialize(
      &ctx->resources,
      ctx->mixer,
      &loscene_context_mixer_format_,
      LORESOURCE_LANGUAGE_JP);

  loshader_set_initialize(
      &ctx->shaders,
      param->width,
      param->height,
      &param->dpi,
      param->max_msaa);

  ctx->scene = loscene_context_create_start_scene_(ctx);
  return ctx;
}

void loscene_context_delete(loscene_context_t* ctx) {
  if (ctx == NULL) return;

  /* Firstly delete the mixer working in other thread. */
  jukebox_mixer_delete(ctx->mixer);

  loscene_delete(ctx->scene);

  loshader_set_deinitialize(&ctx->shaders);
  loresource_set_deinitialize(&ctx->resources);

  locommon_ticker_deinitialize(&ctx->ticker);

  glyphas_context_deinitialize(&ctx->glyphas);

  memory_delete(ctx);
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
