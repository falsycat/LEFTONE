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
#include "core/loshader/set.h"

#include "./type.h"

typedef struct {
  lobullet_type_t type;
  loentity_id_t   owner;
  loentity_id_t   target;

  locommon_position_t basepos;
  vec2_t              size;
  vec4_t              color;

  vec2_t velocity;
  vec2_t acceleration;

  float angle;
  float angular_velocity;

  bool  quiet;
  float knockback;

  loeffect_t effect;

  uint64_t since;
  uint64_t duration;
} lobullet_base_param_t;

typedef struct {
  loentity_bullet_t super;
  bool              used;

  loresource_set_t*        res;
  loshader_set_t*          shaders;
  const locommon_ticker_t* ticker;
  loentity_store_t*        entities;

  lobullet_base_param_t param;

  struct {
    bool  toxic;
    float knockback;
    bool  velocity_calc;

    loshader_bullet_drawer_instance_t instance;
  } cache;
} lobullet_base_t;

void
lobullet_base_initialize(
    lobullet_base_t*         base,
    loresource_set_t*        res,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    loentity_store_t*        entities
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
