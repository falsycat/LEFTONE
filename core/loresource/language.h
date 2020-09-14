#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  LORESOURCE_LANGUAGE_JP,
} loresource_language_t;

const char*
loresource_language_stringify(
    loresource_language_t lang
);

bool
loresource_language_unstringify(
    loresource_language_t* lang,
    const char*            str,
    size_t                 len
);
