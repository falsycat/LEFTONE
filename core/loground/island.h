#pragma once

#include <stdbool.h>

#include "util/math/vector.h"

#include "core/locommon/position.h"

#include "./base.h"

bool
loground_island_update(
    loground_base_t* base
);

void
loground_island_build(
    loground_base_t*           base,
    const locommon_position_t* pos,
    const vec2_t*              size
);
