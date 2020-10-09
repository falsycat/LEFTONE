#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/loentity/entity.h"

#include "./base.h"

typedef struct {
  loentity_id_t target;
} loparticle_aura_param_t;

bool
loparticle_aura_valid(
    const loparticle_aura_param_t* param
);

void
loparticle_aura_param_pack(
    const loparticle_aura_param_t* param,
    msgpack_packer*                packer
);

bool
loparticle_aura_param_unpack(
    loparticle_aura_param_t* param,
    const msgpack_object*    obj
);

void
loparticle_aura_build(
    loparticle_base_t*             base,
    loparticle_type_t              type,
    const loparticle_aura_param_t* param
);

bool
loparticle_aura_guard_update(
    loparticle_base_t* base
);
#define loparticle_aura_guard_build(base, param)  \
    loparticle_aura_build(base, LOPARTICLE_TYPE_AURA_GUARD, param)
#define loparticle_aura_guard_tear_down(base)
#define loparticle_aura_guard_pack_data(base, packer)  \
    loparticle_aura_param_pack(  \
        (const loparticle_aura_param_t*) base->data, packer)
#define loparticle_aura_guard_unpack_data(base, obj)  \
    loparticle_aura_param_unpack(  \
        (loparticle_aura_param_t*) base->data, obj)
