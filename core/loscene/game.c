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

#include "core/lobullet/pool.h"
#include "core/locharacter/pool.h"
#include "core/locommon/counter.h"
#include "core/locommon/ticker.h"
#include "core/loentity/store.h"
#include "core/loground/pool.h"
#include "core/loplayer/camera.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"
#include "core/loworld/environment.h"
#include "core/loworld/generator.h"
#include "core/loworld/poolset.h"
#include "core/loworld/store.h"
#include "core/loworld/template.h"
#include "core/loworld/view.h"

#include "./param.h"
#include "./scene.h"
#include "./title.h"

typedef struct {
  loscene_t header;

  const loscene_param_t*   param;
  const locommon_ticker_t* app_ticker;
  loresource_set_t*        res;
  loshader_set_t*          shaders;

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

  /* temporary parameters */
  bool   updated;
  mat4_t proj;
} loscene_game_t;

#define LOSCENE_GAME_MAX_DELTA_TIME 500

#define LOSCENE_GAME_SCALE 4.0f

#define LOSCENE_GAME_ENTITY_STORE_RESERVE      256
#define LOSCENE_GAME_WORLD_STORE_CHUNK_RESERVE 64
#define LOSCENE_GAME_BULLETS_PER_CHUNK         64

#define LOSCENE_GAME_DATA_BASEPATH "./data/"

#define LOSCENE_GAME_DATA_FILE_PATH  \
    (LOSCENE_GAME_DATA_BASEPATH"game.msgpack")
#define LOSCENE_GAME_WORLD_STORE_BASEPATH  \
    (LOSCENE_GAME_DATA_BASEPATH"world/")

static void loscene_game_build_projection_matrix_(loscene_game_t* s) {
  assert(s != NULL);

  static const float chunk_inch = 16;
  static const float max_scale  = 1/.5f;

  float yscale = s->shaders->dpi.y*chunk_inch/s->shaders->resolution.y*2;
  float xscale = s->shaders->dpi.x*chunk_inch/s->shaders->resolution.x*2;

  if (xscale > max_scale) {
    yscale *= max_scale/xscale;
    xscale  = max_scale;
  }

  s->proj = mat4_scale(xscale, yscale, 1);
}

static void loscene_game_convert_viewport_pos_to_chunk_pos_(
    loscene_game_t* s, locommon_position_t* pos, const vec2_t* vpos) {
  assert(s != NULL);
  assert(locommon_position_valid(pos));

  mat4_t m, inv;
  mat4_mul(&m, &s->proj, &s->player.camera.matrix);
  mat4_inv(&inv, &m);

  vec4_t disp4;
  const vec4_t vpos4 = vec4(vpos->x, vpos->y, 0, 1);
  mat4_mul_vec4(&disp4, &inv, &vpos4);

  vec2_addeq(&pos->fract, &disp4.xy);
  locommon_position_reduce(pos);
}

static bool loscene_game_load_(loscene_game_t* scene) {
  assert(scene != NULL);
  /* ! Please note that world and view objects may be invalid now. ! */

  bool ret = false;

  msgpack_unpacker unpacker;
  if (!msgpack_unpacker_init(&unpacker, 1024)) return false;

  msgpack_unpacked unpacked;
  msgpack_unpacked_init(&unpacked);

  FILE* fp = fopen(LOSCENE_GAME_DATA_FILE_PATH, "rb");
  if (fp == NULL) goto FINALIZE;

  const bool loaded =
      mpkutil_file_unpack_with_unpacker(&unpacked, fp, &unpacker);
  fclose(fp);
  if (!loaded) goto FINALIZE;

  const msgpack_object_map* root = mpkutil_get_map(&unpacked.data);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

  if (!loplayer_unpack(&scene->player, item_("player")) ||
      !loworld_generator_unpack(scene->generator, item_("generator")) ||
      !locommon_ticker_unpack(&scene->ticker, item_("ticker")) ||
      !locommon_counter_unpack(&scene->idgen, item_("idgen"))) {
    goto FINALIZE;
  }

# undef item_

  ret = true;
FINALIZE:
  msgpack_unpacked_destroy(&unpacked);
  msgpack_unpacker_destroy(&unpacker);

  scene->begin_time = scene->ticker.time;
  return ret;
}

static bool loscene_game_save_(loscene_game_t* scene) {
  assert(scene != NULL);

  bool ret = true;

  loworld_view_flush_store(scene->view);
  if (loworld_store_is_error_happened(scene->world)) ret = false;

  FILE* fp = fopen(LOSCENE_GAME_DATA_FILE_PATH, "wb");
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

  loworld_environment_deinitialize(&s->environment);
  loworld_view_delete(s->view);
  loworld_store_delete(s->world);
  loworld_generator_delete(s->generator);

  loentity_store_clear(s->entities);

  locharacter_pool_delete(s->pools.character);
  loplayer_deinitialize(&s->player);

  lobullet_pool_delete(s->pools.bullet);
  loground_pool_delete(s->pools.ground);

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

  const uint64_t t = s->app_ticker->time - s->app_begin_time + s->begin_time;
  locommon_ticker_tick(&s->ticker, t);
  if (s->ticker.delta > LOSCENE_GAME_MAX_DELTA_TIME) {
    fprintf(stderr, "1 tick took too long (%"PRId64" ms)\n", s->ticker.delta);
    return scene;
  }

  locommon_position_t cursor = s->player.camera.pos;
  loscene_game_convert_viewport_pos_to_chunk_pos_(s, &cursor, &input->cursor);

  loplayer_update(&s->player, input, &cursor);
  if (loplayer_menu_is_exit_requested(s->player.menu)) {
    return loscene_title_new(s->param, s->res, s->shaders, s->app_ticker);
  }

  loworld_view_look(s->view, &s->player.camera.pos);
  loworld_view_update(s->view);

  loworld_environment_update(&s->environment);

  s->updated = true;
  return scene;
}

static void loscene_game_draw_(loscene_t* scene) {
  assert(scene != NULL);

  loscene_game_t* s = (typeof(s)) scene;
  if (!s->updated) return;

  const loshader_uniblock_param_t p = {
    .proj = s->proj,
    .cam  = s->player.camera.matrix,
    .pos  = s->player.camera.pos,
    .time = s->ticker.time%60000/1000.0f,
  };
  loshader_uniblock_update_param(s->shaders->uniblock, &p);

  loshader_set_clear_all(s->shaders);

  loworld_environment_draw(&s->environment);
  loworld_view_draw(s->view);
  loplayer_draw(&s->player);

  loshader_set_draw_all(s->shaders);
}

static void loscene_game_execute_tests_(
    const loscene_game_t* scene, const loscene_param_t* param) {
  assert(scene != NULL);
  assert(param != NULL);

  if (param->test.loworld_poolset_packing) {
    loworld_poolset_test_packing(&scene->pools);
  }
  if (param->test.loplayer_packing) {
    loplayer_test_packing(&scene->player);
  }
}

loscene_t* loscene_game_new(
    const loscene_param_t*   param,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    bool                     load) {
  assert(param   != NULL);
  assert(shaders != NULL);
  assert(res     != NULL);
  assert(ticker  != NULL);

  loshader_set_drop_cache(shaders);

  loscene_game_t* scene = memory_new(sizeof(*scene));
  *scene = (typeof(*scene)) {
    .header = {
      .vtable = {
        .delete = loscene_game_delete_,
        .update = loscene_game_update_,
        .draw   = loscene_game_draw_,
      },
    },
    .param          = param,
    .app_ticker     = ticker,
    .res            = res,
    .shaders        = shaders,
    .app_begin_time = ticker->time,
  };
  loscene_game_build_projection_matrix_(scene);

  locommon_counter_initialize(&scene->idgen, 0);
  locommon_ticker_initialize(&scene->ticker, 0);

  scene->entities = loentity_store_new(
      LOSCENE_GAME_ENTITY_STORE_RESERVE);

  scene->pools.ground = loground_pool_new(
      scene->shaders->drawer.ground,
      &scene->idgen,
      LOSCENE_GAME_WORLD_STORE_CHUNK_RESERVE*
          LOWORLD_TEMPLATE_MAX_CHARACTERS_PER_CHUNK);

  scene->pools.bullet = lobullet_pool_new(
      res,
      scene->shaders->drawer.bullet,
      &scene->idgen,
      &scene->ticker,
      scene->entities,
      LOSCENE_GAME_WORLD_STORE_CHUNK_RESERVE*
          LOSCENE_GAME_BULLETS_PER_CHUNK);

  loplayer_initialize(
      &scene->player,
      locommon_counter_count(&scene->idgen),  /* = Absolutely 0 */
      res,
      scene->shaders,
      &scene->ticker,
      scene->pools.bullet,
      scene->entities,
      &scene->proj);
  scene->player.camera.brightness = param->brightness/1000.f;

  scene->pools.character = locharacter_pool_new(
      res,
      scene->shaders->drawer.character,
      &scene->idgen,
      &scene->ticker,
      scene->pools.bullet,
      scene->entities,
      &scene->player,
      LOSCENE_GAME_WORLD_STORE_CHUNK_RESERVE*
          LOWORLD_TEMPLATE_MAX_CHARACTERS_PER_CHUNK);

  scene->generator = loworld_generator_new(
      &scene->pools,
      scene->app_ticker->time*98641  /* = prime number */);

  if (load) {
    if (!loscene_game_load_(scene)) {
      fprintf(stderr, "failed to load game data\n");
    }
  }

  scene->world = loworld_store_new(
      &scene->pools,
      scene->generator,
      LOSCENE_GAME_WORLD_STORE_CHUNK_RESERVE,
      LOSCENE_GAME_WORLD_STORE_BASEPATH,
      strlen(LOSCENE_GAME_WORLD_STORE_BASEPATH));

  scene->view = loworld_view_new(
      scene->world,
      scene->entities,
      &scene->player.camera.pos);

  loworld_environment_initialize(
      &scene->environment,
      res,
      scene->shaders,
      &scene->ticker,
      scene->view,
      &scene->player,
      &param->environment);

  loscene_game_execute_tests_(scene, param);

  return &scene->header;
}
