#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/loentity/entity.h"

#include "./base.h"

typedef struct {
  loentity_id_t ground;
  float         pos;
  float         direction;
} locharacter_scientist_param_t;

bool
locharacter_scientist_update(
    locharacter_base_t* base
);

void
locharacter_scientist_build(
    locharacter_base_t*               base,
    const locharacter_scientist_param_t* param
);

#define locharacter_scientist_tear_down(base)

#define locharacter_scientist_pack_data(base, packer)  \
    msgpack_pack_nil(packer)

#define locharacter_scientist_unpack_data(base, obj)  \
    (obj != NULL)
