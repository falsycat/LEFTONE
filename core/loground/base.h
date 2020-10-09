#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "core/loentity/entity.h"
#include "core/loentity/ground.h"
#include "core/loshader/ground.h"

#include "./type.h"

typedef struct {
  loentity_ground_t super;
  bool              used;

  loshader_ground_drawer_t* drawer;

  struct {
    loground_type_t type;
  } param;

  struct {
    loshader_ground_drawer_instance_t instance;
  } cache;
} loground_base_t;

void
loground_base_initialize(
    loground_base_t*          base,
    loshader_ground_drawer_t* drawer
);

void
loground_base_reinitialize(
    loground_base_t* base,
    loentity_id_t    id
);

void
loground_base_deinitialize(
    loground_base_t* base
);

bool
loground_base_unpack(
    loground_base_t*     base,
    const msgpack_object *obj
);
