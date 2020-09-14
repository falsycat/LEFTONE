#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* DON'T EXPECT THIS ALGORITHM TO BE ONE-WAY */

uint64_t
chaos_abchash(
    const char* str,
    size_t      len
);

bool
chaos_abchash_validate(
    const char* str,
    size_t      len
);
