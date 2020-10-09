#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"
#include "util/statman/statman.h"

#include "core/lobullet/pool.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/recipient.h"
#include "core/loentity/character.h"
#include "core/loentity/entity.h"
#include "core/loentity/ground.h"
#include "core/loentity/store.h"
#include "core/loplayer/player.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./state.h"
#include "./strategy.h"
#include "./type.h"

typedef struct lochara_base_t {
  loentity_character_t super;
  bool                 used;

  loresource_set_t*        res;
  loshader_set_t*          shaders;
  const locommon_ticker_t* ticker;
  loentity_store_t*        entities;
  loplayer_t*              player;
  lobullet_pool_t*         bullet;

  struct {
    lochara_type_t type;

    loeffect_recipient_t recipient;

    statman_state_t state;  /* actual type is lochara_state_t */
    uint64_t        last_state_changed;

    statman_state_t strategy;  /* actual type is lochara_strategy_t */
    uint64_t        last_strategy_changed;

    loentity_id_t ground;
    bool          on_ground;

    vec2_t   direction;
    vec2_t   movement;
    float    gravity;
    float    knockback;
    uint64_t last_knockback;
    uint64_t last_bullet_hit;

    /* some character uses these params for some strategy */
    struct {
      locommon_position_t pos;
      vec2_t              vec;
    } anchor;
  } param;

  struct {
    loentity_ground_t* ground;
    vec2_t             ground_pos;
    loshader_character_drawer_instance_t instance;
  } cache;
} lochara_base_t;

void
lochara_base_initialize(
    lochara_base_t*          base,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities,
    loplayer_t*              player,
    lobullet_pool_t*         bullet
);

void
lochara_base_reinitialize(
    lochara_base_t* base,
    loentity_id_t   id
);

void
lochara_base_deinitialize(
    lochara_base_t* base
);

void
lochara_base_calculate_physics(
    lochara_base_t* base,
    const vec2_t*   size,
    const vec2_t*   offset
);

void
lochara_base_bind_on_ground(
    lochara_base_t* base,
    const vec2_t*   offset
);

bool
lochara_base_affect_bullets(
    lochara_base_t* base
);

bool
lochara_base_unpack(
    lochara_base_t*       base,
    const msgpack_object* obj
);
