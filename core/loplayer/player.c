#include "./player.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include <msgpack.h>

#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/input.h"
#include "core/locommon/position.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"

#include "./camera.h"
#include "./combat.h"
#include "./controller.h"
#include "./event.h"

/* generated serializer */
#include "core/loplayer/crial/player.h"

void loplayer_initialize(
    loplayer_t*              player,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities) {
  assert(player   != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);

  *player = (typeof(*player)) {0};

  loplayer_stance_set_initialize(&player->stances);
  loplayer_stance_set_add(&player->stances, LOPLAYER_STANCE_MISSIONARY);

  loplayer_combat_initialize(&player->combat, ticker, entities);
  loplayer_controller_initialize(&player->controller, ticker);
  loplayer_camera_initialize(&player->camera, screen, ticker);
  loplayer_event_initialize(&player->event, ticker);
  loplayer_popup_initialize(&player->popup);
}

void loplayer_deinitialize(loplayer_t* player) {
  assert(player != NULL);

  loplayer_popup_deinitialize(&player->popup);
  loplayer_event_deinitialize(&player->event);
  loplayer_camera_deinitialize(&player->camera);
  loplayer_controller_deinitialize(&player->controller);
  loplayer_combat_deinitialize(&player->combat);

  loplayer_stance_set_deinitialize(&player->stances);
}

void loplayer_update(
    loplayer_t*                player,
    const locommon_input_t*    input,
    const locommon_position_t* cursor) {
  assert(player != NULL);

  loplayer_controller_handle_input(&player->controller, input, cursor);

  loplayer_combat_drop_dead_attack(&player->combat);
}

void loplayer_pack(const loplayer_t* player, msgpack_packer* packer) {
  assert(player != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, CRIAL_PROPERTY_COUNT_+1);
  CRIAL_SERIALIZER_;

  mpkutil_pack_str(packer, "entity");
  loentity_pack((const loentity_t*) player->entity, packer);
}

bool loplayer_unpack(loplayer_t* player, const msgpack_object* obj) {
  assert(player != NULL);

  const msgpack_object_map* root = mpkutil_get_map(obj);
  if (root == NULL) return false;

  CRIAL_DESERIALIZER_;
  return true;
}
