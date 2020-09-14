#pragma once

#include <stdbool.h>
#include <stddef.h>

/* dont forget to update EACH macro */
typedef enum {
  LOGROUND_TYPE_ISLAND,
} loground_type_t;

#define LOGROUND_TYPE_EACH_(PROC) do {  \
  PROC(ISLAND, island);  \
} while (0)

const char*
loground_type_stringify(
    loground_type_t type
);

bool
loground_type_unstringify(
    loground_type_t* type,
    const char*      v,
    size_t           len
);
