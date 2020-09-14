#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/loentity/entity.h"

#include "./base.h"

bool
locharacter_greedy_scientist_update(
    locharacter_base_t* base
);

void
locharacter_greedy_scientist_build(
    locharacter_base_t* base,
    loentity_id_t       ground
);

void
locharacter_greedy_scientist_tear_down(
    locharacter_base_t* base
);

void
locharacter_greedy_scientist_pack_data(
    const locharacter_base_t* base,
    msgpack_packer*           packer
);

bool
locharacter_greedy_scientist_unpack_data(
    locharacter_base_t*   base,
    const msgpack_object* obj
);
