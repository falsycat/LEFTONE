#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* plz note that all functions are not thread-safe */

struct flasy_t;
typedef struct flasy_t flasy_t;

flasy_t*  /* OWNERSHIP */
flasy_new(
    size_t bufsz,
    size_t hlen
);

void
flasy_delete(
    flasy_t* srv  /* OWNERSHIP */
);

FILE*  /* OWNERSHIP/NULLABLE */
flasy_open_file(
    flasy_t*    srv,
    const char* path,
    bool        binary
);

void
flasy_close_file(
    flasy_t* srv,
    FILE*    fp  /* OWNERSHIP */
);
