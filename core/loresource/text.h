#pragma once

#include "./language.h"

void
loresource_text_optimize(
    loresource_language_t lang
);

const char*
loresource_text_get(
    loresource_language_t lang,
    const char*           key
);
