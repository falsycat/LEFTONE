#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loentity/store.h"
#include "core/loshader/particle.h"

#include "./misc.h"

typedef struct {
  loentity_t super;
  bool       used;

  /* injected deps */
  loshader_particle_drawer_t* drawer;
  const locommon_ticker_t*    ticker;
  loentity_store_t*           entities;

  /* params not to be packed */
  struct {
    loshader_particle_drawer_instance_t instance;
  } cache;

  /* params to be packed */
  loparticle_type_t type;

# define LOPARTICLE_BASE_DATA_MAX_SIZE 256
  uint8_t data[LOPARTICLE_BASE_DATA_MAX_SIZE];
      /* pack function for the type is used */
} loparticle_base_t;

void
loparticle_base_initialize(
    loparticle_base_t*          base,
    loshader_particle_drawer_t* drawer,
    const locommon_ticker_t*    ticker,
    loentity_store_t*           entities
);

void
loparticle_base_reinitialize(
    loparticle_base_t* base,
    loentity_id_t      id
);

void
loparticle_base_deinitialize(
    loparticle_base_t* base
);

bool
loparticle_base_unpack(
    loparticle_base_t*    base,
    const msgpack_object* obj
);
