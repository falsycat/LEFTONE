#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"

typedef struct {
  struct {
    int32_t x, y;
  } chunk;
  vec2_t fract;
} locommon_position_t;

#define locommon_position(chunk_x, chunk_y, fract) \
    ((locommon_position_t) { {chunk_x, chunk_y, }, fract })

#define LOCOMMON_POSITION_CHUNKS_PER_DISTRICT 7

int32_t
locommon_position_get_district_from_chunk(
    int32_t x
);

int32_t
locommon_position_get_chunk_index_in_district(
    int32_t x
);

bool
locommon_position_valid(
    const locommon_position_t* a
);

void
locommon_position_sub(
    vec2_t*                    a,
    const locommon_position_t* b,
    const locommon_position_t* c
);

void
locommon_position_reduce(
    locommon_position_t* a
);

void
locommon_position_pack(
    const locommon_position_t* pos,
    msgpack_packer*            packer
);

bool
locommon_position_unpack(
    locommon_position_t*  pos,
    const msgpack_object* obj
);
