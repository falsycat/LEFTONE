#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <msgpack.h>

#include "util/math/vector.h"

void
mpkutil_pack_bool(
    msgpack_packer* packer,
    bool            v
);

void
mpkutil_pack_str(
    msgpack_packer* packer,
    const char*     str  /* NULL-terminated */
);

void
mpkutil_pack_strn(
    msgpack_packer* packer,
    const char*     str,
    size_t          len
);

void
mpkutil_pack_vec2(
    msgpack_packer* packer,
    const vec2_t*   v
);

void
mpkutil_pack_vec4(
    msgpack_packer* packer,
    const vec4_t*   v
);
