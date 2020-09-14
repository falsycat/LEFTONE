#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "./chunk.h"
#include "./poolset.h"

struct loworld_generator_t;
typedef struct loworld_generator_t loworld_generator_t;

loworld_generator_t*
loworld_generator_new(
    const loworld_poolset_t* pools,
    uint64_t                 seed
);

void
loworld_generator_delete(
    loworld_generator_t* gen
);

void
loworld_generator_randomize(
    loworld_generator_t* gen,
    uint64_t             seed
);

void
loworld_generator_generate(
    const loworld_generator_t* gen,
    loworld_chunk_t*           chunk
);

void
loworld_generator_pack(
    const loworld_generator_t* gen,
    msgpack_packer*            packer
);

bool
loworld_generator_unpack(
    loworld_generator_t*  gen,
    const msgpack_object* obj
);
