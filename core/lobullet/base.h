#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/locommon/ticker.h"
#include "core/loentity/bullet.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"
#include "core/loresource/set.h"
#include "core/loshader/bullet.h"

#include "./misc.h"

typedef enum {
  LOBULLET_BASE_KNOCKBACK_ALGORITHM_VELOCITY,
  LOBULLET_BASE_KNOCKBACK_ALGORITHM_POSITION,
} lobullet_base_knockback_algorithm_t;

typedef struct {
  loentity_bullet_t super;
  bool              used;

  /* injected deps */
  loresource_set_t*         res;
  loshader_bullet_drawer_t* drawer;
  const locommon_ticker_t*  ticker;
  loentity_store_t*         entities;

  /* params not to be packed */
  struct {
    bool       toxic;
    loeffect_t effect;
        /* When toxic is true, apply this effect to characters hit. */

    struct {
      float                               acceleration;
      lobullet_base_knockback_algorithm_t algorithm;
    } knockback;

    loshader_bullet_drawer_instance_t instance;
        /* instance pos is added to draw pos */
  } cache;

  /* params to be packed (includes id) */
  lobullet_type_t type;

# define LOBULLET_BASE_DATA_MAX_SIZE 256
  uint8_t data[LOBULLET_BASE_DATA_MAX_SIZE];
      /* pack function for the type is used */
} lobullet_base_t;

void
lobullet_base_initialize(
    lobullet_base_t*          base,
    loresource_set_t*         res,
    loshader_bullet_drawer_t* drawer,
    const locommon_ticker_t*  ticker,
    loentity_store_t*         entities
);

void
lobullet_base_reinitialize(
    lobullet_base_t* base,
    loentity_id_t    id
);

void
lobullet_base_deinitialize(
    lobullet_base_t* base
);

bool
lobullet_base_unpack(
    lobullet_base_t*      base,
    const msgpack_object* obj
);
