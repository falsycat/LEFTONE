#pragma once

#include <stddef.h>
#include <stdint.h>

size_t
conv_charcode_utf8_to_utf32(
    uint32_t*   c,
    const char* s,
    size_t      len
);
