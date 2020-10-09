#pragma once

#include <stdbool.h>
#include <stddef.h>

/* don't forget to update EACH macro */
typedef enum {
  LOPARTICLE_TYPE_AURA_GUARD,
} loparticle_type_t;

#define LOPARTICLE_TYPE_EACH_(PROC) do {  \
  PROC(AURA_GUARD, aura_guard);  \
} while (0)

const char*
loparticle_type_stringify(
    loparticle_type_t type
);

bool
loparticle_type_unstringify(
    loparticle_type_t* type,
    const char*        v,
    size_t             len
);
