#pragma once

#include <stdint.h>

uint64_t
chaos_xorshift(
    uint64_t seed
);

float
chaos_xorshift_fract(
    uint64_t  seed,
    uint64_t* next_seed  /* NULLABLE */
);
