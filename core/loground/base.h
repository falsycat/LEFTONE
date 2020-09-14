#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "core/loentity/entity.h"
#include "core/loentity/ground.h"
#include "core/loshader/ground.h"

#include "./misc.h"

typedef struct {
  loentity_ground_t super;
  bool              used;

  /* injected deps */
  loshader_ground_drawer_t* drawer;

  /* params not to be packed */
  struct {
    loshader_ground_drawer_instance_t instance;
  } cache;

  /* params to be packed (includes id, pos, and size) */
  loground_type_t type;

# define LOGROUND_BASE_DATA_MAX_SIZE 256
  uint8_t data[LOGROUND_BASE_DATA_MAX_SIZE];
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
