#include "./game.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <GL/glew.h>
#include <msgpack.h>
#include <msgpack/fbuffer.h>

#include "util/math/matrix.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"
#include "util/mpkutil/file.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/lochara/pool.h"
#include "core/lochara/player.h"
#include "core/locommon/counter.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loentity/store.h"
#include "core/loplayer/player.h"
#include "core/loui/ui.h"
#include "core/loworld/environment.h"
#include "core/loworld/generator.h"
#include "core/loworld/poolset.h"
#include "core/loworld/store.h"
#include "core/loworld/view.h"

#include "./context.h"
#include "./param.h"
#include "./scene.h"
#include "./title.h"

typedef struct {
  loscene_t header;

  loscene_context_t* ctx;

  uint64_t app_begin_time;
  uint64_t begin_time;

  locommon_counter_t idgen;
  locommon_ticker_t  ticker;

  loentity_store_t* entities;
  loplayer_t        player;
  loworld_poolset_t pools;

  loworld_generator_t*  generator;
  loworld_store_t*      world;
  loworld_view_t*       view;
  loworld_environment_t environment;

  loui_t ui;

  bool   updated;
  mat4_t proj;
  mat4_t camera;
} loscene_game_t;

#define MAX_DELTA_TIME_ 500

#define ENTITY_STORE_RESERVE_      256
#define WORLD_STORE_CHUNK_RESERVE_ 64

#define DATA_BASEPATH_        "./data/"
#define DATA_FILE_PATH_       DATA_BASEPATH_"game.msgpack"
#define WORLD_STORE_BASEPATH_ DATA_BASEPATH_"world/"

static void loscene_game_convert_viewport_pos_to_chunk_pos_(
    loscene_game_t* s, locommon_position_t* pos, const vec2_t* vpos) {
  assert(s != NULL);
  assert(locommon_position_valid(pos));

  mat4_t m, inv;
  mat4_mul(&m, &s->proj, &mat4_identity());
  mat4_inv(&inv, &m);

  vec4_t disp4;
  const vec4_t vpos4 = vec4(vpos->x, vpos->y, 0, 1);
  mat4_mul_vec4(&disp4, &inv, &vpos4);

  vec2_addeq(&pos->fract, &disp4.xy);
  locommon_position_reduce(pos);
}

static bool loscene_game_load_(loscene_game_t* s) {
  assert(s != NULL);

  bool ret = false;

  msgpack_unpacker unpacker;
  if (!msgpack_unpacker_init(&unpacker, 1024)) return false;

  msgpack_unpacked unpacked;
  msgpack_unpacked_init(&unpacked);

  FILE* fp = fopen(DATA_FILE_PATH_, "rb");
  if (fp == NULL) goto FINALIZE;

  const bool loaded =
      mpkutil_file_unpack_with_unpacker(&unpacked, fp, &unpacker);
  fclose(fp);
  if (!loaded) goto FINALIZE;

  const msgpack_object_map* root = mpkutil_get_map(&unpacked.data);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

  const msgpack_object* player = item_("player");
  s->player.entity = lochara_pool_unpack_item(
      s->pools.chara,
      mpkutil_get_map_item_by_str(mpkutil_get_map(player), "entity"));
  if (s->player.entity == NULL) goto FINALIZE;

  if (s->player.entity->param.type != LOCHARA_TYPE_PLAYER) {
    loentity_delete(&s->player.entity->super.super);
    s->player.entity = NULL;
    goto FINALIZE;
  }

  if (!loplayer_unpack(&s->player, player) ||
      !loworld_generator_unpack(s->generator, item_("generator")) ||
      !locommon_ticker_unpack(&s->ticker, item_("ticker")) ||
      !locommon_counter_unpack(&s->idgen, item_("idgen"))) {
    goto FINALIZE;
  }

# undef item_

  ret = true;
FINALIZE:
  msgpack_unpacked_destroy(&unpacked);
  msgpack_unpacker_destroy(&unpacker);

  s->begin_time = s->ticker.time;
  return ret;
}

static bool loscene_game_save_(loscene_game_t* scene) {
  assert(scene != NULL);

  bool ret = true;

  loworld_view_flush_store(scene->view);
  if (loworld_store_is_error_happened(scene->world)) ret = false;

  FILE* fp = fopen(DATA_FILE_PATH_, "wb");
  if (fp == NULL) return false;

  msgpack_packer pk;
  msgpack_packer_init(&pk, fp, msgpack_fbuffer_write);

  msgpack_pack_map(&pk, 4);

  mpkutil_pack_str(&pk, "player");
  loplayer_pack(&scene->player, &pk);

  mpkutil_pack_str(&pk, "generator");
  loworld_generator_pack(scene->generator, &pk);

  mpkutil_pack_str(&pk, "ticker");
  locommon_ticker_pack(&scene->ticker, &pk);

  mpkutil_pack_str(&pk, "idgen");
  locommon_counter_pack(&scene->idgen, &pk);

  fclose(fp);
  return ret;
}

static void loscene_game_delete_(loscene_t* scene) {
  if (scene == NULL) return;

  loscene_game_t* s = (typeof(s)) scene;
  if (!loscene_game_save_(s)) {
    fprintf(stderr, "failed to save game data\n");
  }

  loui_deinitialize(&s->ui);

  loworld_environment_deinitialize(&s->environment);
  loworld_view_delete(s->view);
  loworld_store_delete(s->world);
  loworld_generator_delete(s->generator);

  loentity_store_clear(s->entities);
  loworld_poolset_deinitialize(&s->pools);
  loplayer_deinitialize(&s->player);
  loentity_store_delete(s->entities);

  locommon_ticker_deinitialize(&s->ticker);
  locommon_counter_deinitialize(&s->idgen);

  memory_delete(scene);
}

static loscene_t* loscene_game_update_(
    loscene_t* scene, const locommon_input_t* input) {
  assert(scene != NULL);
  assert(input != NULL);

  loscene_game_t* s = (typeof(s)) scene;
  s->updated = false;

  const uint64_t t =
      s->ctx->ticker.time - s->app_begin_time + s->begin_time;
  locommon_ticker_tick(&s->ticker, t);
  if (s->ticker.delta > MAX_DELTA_TIME_) {
    fprintf(stderr, "1 tick took too long (%"PRId64" ms)\n", s->ticker.delta);
    return scene;
  }

  s->player.camera.base_brightness = s->ctx->param.brightness;
  loplayer_camera_build_matrix(&s->player.camera, &s->camera);

  locommon_position_t cursor = s->player.camera.pos;
  loscene_game_convert_viewport_pos_to_chunk_pos_(s, &cursor, &input->cursor);

  const bool grabbed = loui_is_grabbing_input(&s->ui);
  loplayer_update(
      &s->player,
      grabbed? NULL: input,
      grabbed? NULL: &cursor);

  loworld_view_look(s->view, &s->player.camera.pos);
  loworld_view_update(s->view);

  loworld_environment_update(&s->environment);

  loui_update(&s->ui, input);
  if (s->ui.menu.request_exit) return loscene_title_new(s->ctx);

  s->updated = true;
  return scene;
}

static void loscene_game_draw_(loscene_t* scene) {
  assert(scene != NULL);

  loscene_game_t* s = (typeof(s)) scene;
  if (!s->updated) return;

  loshader_posteffect_drawer_set_param(
      &s->ctx->shaders.drawer.posteffect,
      &(loshader_posteffect_drawer_param_t) { .brightness_whole = 1, });

  loshader_uniblock_update_param(
      &s->ctx->shaders.uniblock,
      &(loshader_uniblock_param_t) {
        .proj = s->proj,
        .cam  = s->camera,
        .pos  = s->player.camera.pos,
        .time = s->ticker.time%60000/1000.f,
      });

  loshader_set_clear_all(&s->ctx->shaders);

  s->ctx->shaders.drawer.pixsort.intensity = s->player.camera.pixsort;
  loshader_posteffect_drawer_set_param(
      &s->ctx->shaders.drawer.posteffect, &s->player.camera.posteffect);

  loworld_environment_draw(&s->environment);
  loworld_view_draw(s->view);

  loui_draw(&s->ui);

  loshader_set_draw_all(&s->ctx->shaders);
}

loscene_t* loscene_game_new(
    loscene_context_t* ctx,
    bool               load) {
  assert(ctx != NULL);

  loshader_set_drop_cache(&ctx->shaders);

  loscene_game_t* s = memory_new(sizeof(*s));
  *s = (typeof(*s)) {
    .header = {
      .vtable = {
        .delete = loscene_game_delete_,
        .update = loscene_game_update_,
        .draw   = loscene_game_draw_,
      },
    },
    .ctx            = ctx,
    .app_begin_time = ctx->ticker.time,
  };
  locommon_screen_build_projection_matrix(&s->ctx->screen, &s->proj);

  locommon_counter_initialize(&s->idgen, 0);
  locommon_ticker_initialize(&s->ticker, 0);

  s->entities = loentity_store_new(ENTITY_STORE_RESERVE_);

  loplayer_initialize(
      &s->player,
      &s->ctx->screen,
      &s->ticker,
      s->entities);

  loworld_poolset_initialize(
      &s->pools,
      &s->ctx->resources,
      &s->ctx->shaders,
      &s->idgen,
      &s->ticker,
      s->entities,
      &s->player,
      WORLD_STORE_CHUNK_RESERVE_);

  /* id is always 0 */

  s->generator = loworld_generator_new(
      &s->pools,
      s->ctx->ticker.time*98641  /* = prime number */);

  if (load) {
    load = loscene_game_load_(s);
    if (!load) fprintf(stderr, "failed to load game data\n");
  }
  if (!load) {
    locommon_counter_reset(&s->idgen);
    s->player.entity = lochara_pool_create(s->pools.chara);
    lochara_player_build(s->player.entity);
  }
  s->player.entity->super.super.dont_save = true;
  loentity_store_add(s->entities, &s->player.entity->super.super);

  s->world = loworld_store_new(
      s->ctx->flasy,
      &s->pools,
      s->generator,
      WORLD_STORE_CHUNK_RESERVE_,
      WORLD_STORE_BASEPATH_,
      strlen(WORLD_STORE_BASEPATH_));

  s->view = loworld_view_new(
      s->world,
      s->entities,
      &locommon_position(0, 0, vec2(.5f, .5f)));

  loworld_environment_initialize(
      &s->environment,
      &s->ctx->resources,
      &s->ctx->shaders,
      &s->ticker,
      &s->player,
      s->view,
      &s->ctx->param.environment);

  loui_initialize(
      &s->ui,
      &s->ctx->resources,
      &s->ctx->shaders,
      &s->ctx->screen,
      &s->ticker,
      &s->player,
      s->view);

  return &s->header;
}
