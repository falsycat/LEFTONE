#include "./player.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <msgpack.h>
#include <msgpack/sbuffer.h>

#include "util/math/matrix.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/lobullet/pool.h"
#include "core/locommon/input.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"
#include "core/loshader/hud_bar.h"
#include "core/loshader/posteffect.h"
#include "core/loshader/set.h"

#include "./action.h"
#include "./camera.h"
#include "./combat.h"
#include "./controller.h"
#include "./entity.h"
#include "./event.h"
#include "./hud.h"
#include "./menu.h"
#include "./status.h"

#define LOPLAYER_COMBAT_ATTACK_RESERVE 32

void loplayer_initialize(
    loplayer_t*              player,
    loentity_id_t            id,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    lobullet_pool_t*         bullets,
    loentity_store_t*        entities,
    const mat4_t*            proj) {
  assert(player   != NULL);
  assert(res      != NULL);
  assert(shaders  != NULL);
  assert(ticker   != NULL);
  assert(bullets  != NULL);
  assert(entities != NULL);
  assert(mat4_valid(proj));

  *player = (typeof(*player)) {
    .shaders = shaders,
  };

  player->event = loplayer_event_new(
      res,
      shaders);

  loplayer_status_initialize(&player->status, res, ticker);

  loplayer_entity_initialize(
      &player->entity,
      id,
      res->sound,
      shaders->drawer.character,
      ticker,
      entities,
      player->event,
      &player->status);
  loentity_store_add(entities, &player->entity.super.super);

  player->combat = loplayer_combat_new(
      res->sound,
      shaders->drawer.combat_ring,
      ticker,
      entities,
      &player->status,
      &player->entity,
      LOPLAYER_COMBAT_ATTACK_RESERVE);

  loplayer_controller_initialize(&player->controller);

  loplayer_camera_initialize(
      &player->camera,
      shaders,
      ticker,
      player->event,
      &player->status,
      &player->entity,
      proj);
  player->hud = loplayer_hud_new(
      res,
      shaders,
      ticker,
      player->event,
      &player->status,
      &player->entity);
  player->menu = loplayer_menu_new(
      res,
      shaders,
      ticker,
      &player->status,
      &player->controller);

  player->action = loplayer_action_new(
      res,
      ticker,
      bullets,
      entities,
      player->event,
      &player->status,
      &player->entity,
      player->combat,
      &player->controller,
      &player->camera,
      player->hud,
      player->menu);
}

void loplayer_deinitialize(loplayer_t* player) {
  assert(player != NULL);

  loplayer_action_delete(player->action);

  loplayer_menu_delete(player->menu);
  loplayer_hud_delete(player->hud);
  loplayer_camera_deinitialize(&player->camera);

  loplayer_controller_deinitialize(&player->controller);

  loplayer_combat_delete(player->combat);

  loplayer_entity_deinitialize(&player->entity);
  loplayer_status_deinitialize(&player->status);

  loplayer_event_delete(player->event);
}

void loplayer_popup(loplayer_t* player, const char* title, const char* text) {
  assert(player != NULL);

  loplayer_action_start_menu_popup_state(player->action);
  loplayer_menu_popup(player->menu, title, text);
}

bool loplayer_attack(
    loplayer_t* player, const loplayer_combat_attack_t* attack) {
  assert(player != NULL);
  assert(attack != NULL);

  return loplayer_combat_add_attack(player->combat, attack);
}

void loplayer_touch_encephalon(loplayer_t* player) {
  assert(player != NULL);

  loplayer_status_set_respawn_position(
      &player->status, &player->entity.super.super.pos);
  loplayer_status_reset(&player->status);
}

void loplayer_gain_stance(loplayer_t* player, loeffect_stance_id_t id) {
  assert(player != NULL);

  if (!loplayer_status_add_stance(&player->status, id)) return;

  loplayer_action_start_menu_popup_state(player->action);
  loplayer_menu_show_status_with_stance_highlighted(player->menu, id);
}

void loplayer_gain_faith(loplayer_t* player, float amount) {
  assert(player != NULL);

  player->status.recipient.faith += amount;
}

void loplayer_update(
    loplayer_t*                player,
    const locommon_input_t*    input,
    const locommon_position_t* cursor) {
  assert(player != NULL);
  assert(input  != NULL);
  assert(locommon_position_valid(cursor));

  loplayer_status_update(&player->status);
  /* entity is updated through entity store. */

  loplayer_combat_update(player->combat);

  loplayer_camera_update(&player->camera);
  loplayer_hud_update(player->hud);
  loplayer_menu_update(player->menu);

  loplayer_controller_update(&player->controller, input, cursor);
  loplayer_action_execute(player->action);
}

void loplayer_draw(const loplayer_t* player) {
  assert(player != NULL);

  loplayer_camera_draw(&player->camera);
  loplayer_event_draw(player->event);

  loplayer_combat_draw_ui(player->combat);
  loplayer_hud_draw_ui(player->hud);
  loplayer_menu_draw_ui(player->menu);
}

void loplayer_pack(const loplayer_t* player, msgpack_packer* packer) {
  assert(player != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 6);

  mpkutil_pack_str(packer, "status");
  loplayer_status_pack(&player->status, packer);

  mpkutil_pack_str(packer, "entity");
  loplayer_entity_pack(&player->entity, packer);

  mpkutil_pack_str(packer, "combat");
  loplayer_combat_pack(player->combat, packer);

  mpkutil_pack_str(packer, "camera");
  loplayer_camera_pack(&player->camera, packer);

  mpkutil_pack_str(packer, "menu");
  loplayer_menu_pack(player->menu, packer);

  mpkutil_pack_str(packer, "action");
  loplayer_action_pack(player->action, packer);
}

bool loplayer_unpack(loplayer_t* player, const msgpack_object* obj) {
  assert(player != NULL);

  if (obj == NULL) return false;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)
  if (!loplayer_status_unpack(&player->status, item_("status")) ||
      !loplayer_entity_unpack(&player->entity, item_("entity")) ||
      !loplayer_combat_unpack( player->combat, item_("combat")) ||
      !loplayer_camera_unpack(&player->camera, item_("camera")) ||
      !loplayer_menu_unpack  ( player->menu,   item_("menu"  )) ||
      !loplayer_action_unpack( player->action, item_("action"))) {
    return false;
  }
# undef item_
  return true;
}

void loplayer_test_packing(const loplayer_t* player) {
  assert(player != NULL);

  msgpack_sbuffer buf;
  msgpack_sbuffer_init(&buf);

  msgpack_packer pk;
  msgpack_packer_init(&pk, &buf, msgpack_sbuffer_write);

  loplayer_pack(player, &pk);

  msgpack_unpacked upk;
  msgpack_unpacked_init(&upk);

  size_t off = 0;
  const msgpack_unpack_return r =
      msgpack_unpack_next(&upk, buf.data, buf.size, &off);
  if (r != MSGPACK_UNPACK_SUCCESS) {
    fprintf(stderr, "player: invalid msgpack format\n");
    abort();
  }
  if (!loplayer_unpack((loplayer_t*) player, &upk.data)) {
    fprintf(stderr, "player: failed to unpack\n");
    abort();
  }
  printf("player: packing test passed\n");
  msgpack_unpacked_destroy(&upk);
  msgpack_sbuffer_destroy(&buf);
}
