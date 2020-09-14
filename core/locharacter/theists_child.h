#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/loentity/entity.h"

#include "./base.h"

bool
locharacter_theists_child_update(
    locharacter_base_t* base
);

void
locharacter_theists_child_build(
    locharacter_base_t* base,
    loentity_id_t       ground
);

void
locharacter_theists_child_tear_down(
    locharacter_base_t* base
);

void
locharacter_theists_child_pack_data(
    const locharacter_base_t* base,
    msgpack_packer*           packer
);

bool
locharacter_theists_child_unpack_data(
    locharacter_base_t*   base,
    const msgpack_object* obj
);
