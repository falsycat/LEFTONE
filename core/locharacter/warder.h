#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/loentity/entity.h"

#include "./base.h"

typedef struct {
  loentity_id_t ground;
  float         pos;
} locharacter_warder_param_t;

bool
locharacter_warder_update(
    locharacter_base_t* base
);

void
locharacter_warder_build(
    locharacter_base_t*               base,
    const locharacter_warder_param_t* param
);

#define locharacter_warder_tear_down(base)

#define locharacter_warder_pack_data(base, packer)  \
    msgpack_pack_nil(packer)

#define locharacter_warder_unpack_data(base, obj)  \
    (obj != NULL)
