#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <msgpack.h>

typedef struct {
  size_t next;
} locommon_counter_t;

void
locommon_counter_initialize(
    locommon_counter_t* counter,
    size_t              first
);

void
locommon_counter_deinitialize(
    locommon_counter_t* counter
);

size_t
locommon_counter_count(
    locommon_counter_t* counter
);

void
locommon_counter_reset(
    locommon_counter_t* counter
);

void
locommon_counter_pack(
    const locommon_counter_t* counter,
    msgpack_packer*           packer
);

bool
locommon_counter_unpack(
    locommon_counter_t*   counter,
    const msgpack_object* obj  /* NULLABLE */
);
