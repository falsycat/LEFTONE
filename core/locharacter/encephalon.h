#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/loentity/entity.h"

#include "./base.h"

bool
locharacter_encephalon_update(
    locharacter_base_t* base
);

void
locharacter_encephalon_build(
    locharacter_base_t* base,
    loentity_id_t       ground
);

#define locharacter_encephalon_tear_down(base)

void
locharacter_encephalon_pack_data(
    const locharacter_base_t* base,
    msgpack_packer*           packer
);

bool
locharacter_encephalon_unpack_data(
    locharacter_base_t*   base,
    const msgpack_object* obj
);
