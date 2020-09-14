#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"

bool
mpkutil_get_bool(
    const msgpack_object* obj,  /* NULLABLE */
    bool*                 b     /* NULLABLE */
);

bool
mpkutil_get_int(
    const msgpack_object* obj,  /* NULLABLE */
    intmax_t*             i     /* NULLABLE */
);

#define decl_mpkutil_get_intN_(N)  \
    bool  \
    mpkutil_get_int##N(  \
        const msgpack_object* obj,  \
        int##N##_t*           i  \
    );
decl_mpkutil_get_intN_(8);
decl_mpkutil_get_intN_(16);
decl_mpkutil_get_intN_(32);
decl_mpkutil_get_intN_(64);
#undef decl_mpkutil_get_intN_

bool
mpkutil_get_uint(
    const msgpack_object* obj,  /* NULLABLE */
    uintmax_t*            i     /* NULLABLE */
);

#define decl_mpkutil_get_uintN_(N)  \
    bool  \
    mpkutil_get_uint##N(  \
        const msgpack_object* obj,  \
        uint##N##_t*          i  \
    );
decl_mpkutil_get_uintN_(8);
decl_mpkutil_get_uintN_(16);
decl_mpkutil_get_uintN_(32);
decl_mpkutil_get_uintN_(64);
#undef decl_mpkutil_get_uintN_

bool
mpkutil_get_float(
    const msgpack_object* obj,  /* NULLABLE */
    float*                f     /* NULLABLE */
);

bool
mpkutil_get_str(
    const msgpack_object* obj,  /* NULLABLE */
    const char**          s,    /* NULLABLE */
    size_t*               len   /* NULLABLE */
);

bool
mpkutil_get_vec2(
    const msgpack_object* obj,  /* NULLABLE */
    vec2_t*               v     /* NULLABLE */
);

bool
mpkutil_get_vec4(
    const msgpack_object* obj,  /* NULLABLE */
    vec4_t*               v     /* NULLABLE */
);

const msgpack_object_array*  /* NULLABLE */
mpkutil_get_array(
    const msgpack_object* obj  /* NULLABLE */
);

const msgpack_object_map*  /* NULLABLE */
mpkutil_get_map(
    const msgpack_object* obj  /* NULLABLE */
);

const msgpack_object*  /* NULLABLE */
mpkutil_get_map_item_by_str(
    const msgpack_object_map* map,  /* NULLABLE */
    const char*               name  /* NULL-terminated */
);
