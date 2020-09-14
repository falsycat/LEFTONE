#pragma once

#include <stdbool.h>

#include <msgpack.h>

/* used in some macro templates */
typedef struct {} locommon_null_t;

void
locommon_null_pack(
    const locommon_null_t* null,
    msgpack_packer*        packer
);

bool
locommon_null_unpack(
    locommon_null_t*      null,
    const msgpack_object* obj
);
