#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

typedef struct {
  float amount;
} loeffect_generic_immediate_param_t;

typedef struct {
  uint64_t begin;
  uint64_t duration;
  float    amount;
} loeffect_generic_lasting_param_t;

void
loeffect_generic_immediate_param_pack(
    const loeffect_generic_immediate_param_t* param,
    msgpack_packer*                           packer
);

bool
loeffect_generic_immediate_param_unpack(
    loeffect_generic_immediate_param_t* param,
    const msgpack_object*               obj
);

void
loeffect_generic_lasting_param_pack(
    const loeffect_generic_lasting_param_t* param,
    msgpack_packer*                         packer
);

bool
loeffect_generic_lasting_param_unpack(
    loeffect_generic_lasting_param_t* param,
    const msgpack_object*             obj
);
