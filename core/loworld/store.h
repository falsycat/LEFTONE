#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/flasy/flasy.h"

#include "./chunk.h"
#include "./generator.h"
#include "./poolset.h"

struct loworld_store_t;
typedef struct loworld_store_t loworld_store_t;

/* TODO(catfoot): make it possible to specify a path to chunk dir */
loworld_store_t*  /* OWNERSHIP */
loworld_store_new(
    flasy_t*                   flasy,
    const loworld_poolset_t*   pools,
    const loworld_generator_t* gen,
    size_t                     chunks_length,
    const char*                basepath,  /* must be terminated with slash */
    size_t                     basepath_length
);

void
loworld_store_delete(
    loworld_store_t* store  /* OWNERSHIP */
);

loworld_chunk_t*  /* NULLABLE */
loworld_store_load_chunk(
    loworld_store_t* store,
    int32_t          chunk_x,
    int32_t          chunk_y
);

void
loworld_store_unload_chunk(
    loworld_store_t* store,
    loworld_chunk_t* chunk
);

/* If there is an instance of loworld_view_t, this function may flush broken
   chunks. So use loworld_view_flush_store function insteadly. */
void
loworld_store_flush(
    loworld_store_t* store
);

bool
loworld_store_is_error_happened(
    const loworld_store_t* store
);
