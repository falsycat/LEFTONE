#pragma once

#include <stdbool.h>

#include "core/loentity/ground.h"

#include "./base.h"

bool
lochara_big_warder_update(
    lochara_base_t* base
);

void
lochara_big_warder_build(
    lochara_base_t*    base,
    loentity_ground_t* gnd
);
