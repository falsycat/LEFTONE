#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/loentity/entity.h"

#include "./base.h"

typedef struct {
  loentity_id_t ground;
  float         pos;
  float         direction;
} locharacter_cavia_param_t;

bool
locharacter_cavia_update(
    locharacter_base_t* base
);

void
locharacter_cavia_build(
    locharacter_base_t*              base,
    const locharacter_cavia_param_t* param
);

#define locharacter_cavia_tear_down(base)

#define locharacter_cavia_pack_data(base, packer)  \
    msgpack_pack_nil(packer)

#define locharacter_cavia_unpack_data(base, obj)  \
    (obj != NULL)
