#pragma once

#include <stdbool.h>

#include "core/loentity/ground.h"

#include "./base.h"

bool
lochara_encephalon_update(
    lochara_base_t* base
);

void
lochara_encephalon_build(
    lochara_base_t*          base,
    const loentity_ground_t* gnd
);
