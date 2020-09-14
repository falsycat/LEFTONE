#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/container/array.h"

#include "core/loentity/entity.h"

#include "./poolset.h"

#define LOWORLD_CHUNK_FILENAME_MAX 64

/* dont forget to change EACH macro */
typedef enum {
  LOWORLD_CHUNK_BIOME_METAPHYSICAL_GATE,
  LOWORLD_CHUNK_BIOME_CAVIAS_CAMP,
  LOWORLD_CHUNK_BIOME_LABORATORY,
  LOWORLD_CHUNK_BIOME_BOSS_THEISTS_CHILD,
  LOWORLD_CHUNK_BIOME_BOSS_BIG_WARDER,
  LOWORLD_CHUNK_BIOME_BOSS_GREEDY_SCIENTIST,
} loworld_chunk_biome_t;

#define LOWORLD_CHUNK_BIOME_EACH_(PROC) do {  \
  PROC(METAPHYSICAL_GATE,     metaphysical-gate);  \
  PROC(CAVIAS_CAMP,           cavias-camp);  \
  PROC(LABORATORY,            laboratory);  \
  PROC(BOSS_THEISTS_CHILD,    boss-theists-child);  \
  PROC(BOSS_BIG_WARDER,       boss-big-warder);  \
  PROC(BOSS_GREEDY_SCIENTIST, boss-greedy-scientist);  \
} while (0)

typedef struct {
  struct {
    int32_t x;
    int32_t y;
  } pos;

  loworld_chunk_biome_t biome;

  CONTAINER_ARRAY loentity_t** entities;
} loworld_chunk_t;

const char*
loworld_chunk_biome_stringify(
    loworld_chunk_biome_t biome
);

bool
loworld_chunk_biome_unstringify(
    loworld_chunk_biome_t* biome,
    const char*            str,
    size_t                 len
);

void
loworld_chunk_initialize(
    loworld_chunk_t* chunk
);

void
loworld_chunk_deinitialize(
    loworld_chunk_t* chunk
);

void
loworld_chunk_add_entity(
    loworld_chunk_t* chunk,
    loentity_t*      entity  /* OWNERSHIP */
);

void
loworld_chunk_clear(
    loworld_chunk_t* chunk
);

void
loworld_chunk_pack(
    const loworld_chunk_t* chunk,
    msgpack_packer*        packer
);

bool
loworld_chunk_unpack(
    loworld_chunk_t*         chunk,  /* should be cleared */
    const msgpack_object*    obj,
    const loworld_poolset_t* pools
);

void
loworld_chunk_build_filename(
    const loworld_chunk_t* chunk,
    char*                  filename,
    size_t                 length
);
