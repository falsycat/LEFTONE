#include "./store.h"

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <msgpack.h>
#include <msgpack/fbuffer.h>

#include "util/flasy/flasy.h"
#include "util/memory/memory.h"
#include "util/mpkutil/file.h"

#include "./chunk.h"
#include "./generator.h"
#include "./poolset.h"

#define LOWORLD_STORE_PATH_MAX_LENGTH      256
#define LOWORLD_STORE_UNPACKER_BUFFER_SIZE (1024*1024)

typedef struct {
  loworld_chunk_t data;

  bool loaded;
  bool used;

  uint64_t last_tick;
} loworld_store_chunk_t;

struct loworld_store_t {
  char   path[LOWORLD_STORE_PATH_MAX_LENGTH+LOWORLD_CHUNK_FILENAME_MAX];
  size_t basepath_length;

  flasy_t*                   flasy;
  const loworld_poolset_t*   pools;
  const loworld_generator_t* generator;

  msgpack_unpacker unpacker;
  msgpack_unpacked unpacked;

  uint64_t tick;
  bool     error;

  size_t                chunks_length;
  loworld_store_chunk_t chunks[1];
};

static bool loworld_store_find_chunk_index_(
    const loworld_store_t* store, size_t* index, int32_t x, int32_t y) {
  assert(store != NULL);
  assert(index != NULL);

  for (size_t i = 0; i < store->chunks_length; ++i) {
    if (!store->chunks[i].loaded) continue;

    const loworld_chunk_t* chunk = &store->chunks[i].data;
    if (chunk->pos.x == x && chunk->pos.y == y) {
      *index = i;
      return true;
    }
  }
  return false;
}

static bool loworld_store_find_unused_chunk_index_(
    const loworld_store_t* store, size_t* index) {
  assert(store != NULL);
  assert(index != NULL);

  for (size_t i = 0; i < store->chunks_length; ++i) {
    if (!store->chunks[i].loaded) {
      *index = i;
      return true;
    }
  }

  bool     found  = false;
  uint64_t oldest = UINT64_MAX;
  for (size_t i = 0; i < store->chunks_length; ++i) {
    const loworld_store_chunk_t* chunk = &store->chunks[i];
    if (!chunk->used && chunk->last_tick <= oldest) {
      *index = i;
      oldest = chunk->last_tick;
      found  = true;
    }
  }
  return found;
}

/* Builds filename on the store->path as zero-terminated string. */
static void loworld_store_build_chunk_filename_(
    loworld_store_t* store, const loworld_chunk_t* chunk) {
  assert(store != NULL);
  assert(chunk != NULL);

  loworld_chunk_build_filename(chunk,
      &store->path[store->basepath_length], LOWORLD_CHUNK_FILENAME_MAX);
}

static bool loworld_store_load_chunk_from_file_(
    loworld_store_t* store, loworld_chunk_t* chunk) {
  assert(store != NULL);
  assert(chunk != NULL);

  loworld_store_build_chunk_filename_(store, chunk);

  FILE* fp = fopen(store->path, "rb");
  if (fp == NULL) return false;

  bool success = false;

  msgpack_unpacker_reset(&store->unpacker);
  if (mpkutil_file_unpack_with_unpacker(
        &store->unpacked, fp, &store->unpacker)) {
    loworld_chunk_clear(chunk);
    success = loworld_chunk_unpack(chunk, &store->unpacked.data, store->pools);
  }
  fclose(fp);

  if (!success) {
    fprintf(stderr,
        "failed to load chunk (%"PRId32", %"PRId32")\n",
        chunk->pos.x, chunk->pos.y);
  }
  return success;
}

static bool loworld_store_save_chunk_to_file_(
    loworld_store_t* store, const loworld_chunk_t* chunk) {
  assert(store != NULL);
  assert(chunk != NULL);

  loworld_store_build_chunk_filename_(store, chunk);

  FILE* fp = flasy_open_file(store->flasy, store->path, true);
  if (fp == NULL) return false;

  msgpack_packer packer;
  msgpack_packer_init(&packer, fp, msgpack_fbuffer_write);

  loworld_chunk_pack(chunk, &packer);

  const bool success = (ferror(fp) == 0);
  flasy_close_file(store->flasy, fp);
  return success;
}

loworld_store_t* loworld_store_new(
    flasy_t*                   flasy,
    const loworld_poolset_t*   pools,
    const loworld_generator_t* generator,
    size_t                     chunks_length,
    const char*                basepath,
    size_t                     basepath_length) {
  assert(flasy     != NULL);
  assert(pools     != NULL);
  assert(generator != NULL);
  assert(chunks_length > 0);

  if (basepath_length >= LOWORLD_STORE_PATH_MAX_LENGTH) {
    fprintf(stderr, "too long path name\n");
    abort();
  }

  loworld_store_t* store = memory_new(
      sizeof(*store) + (chunks_length-1)*sizeof(store->chunks[0]));
  *store = (typeof(*store)) {
    .basepath_length = basepath_length,
    .flasy           = flasy,
    .pools           = pools,
    .generator       = generator,
    .chunks_length   = chunks_length,
  };
  strncpy(store->path, basepath, LOWORLD_STORE_PATH_MAX_LENGTH);

  if (!msgpack_unpacker_init(
        &store->unpacker, LOWORLD_STORE_UNPACKER_BUFFER_SIZE)) {
    fprintf(stderr, "failed to initialize unpacker\n");
    abort();
  }
  msgpack_unpacked_init(&store->unpacked);

  for (size_t i = 0; i < store->chunks_length; ++i) {
    loworld_store_chunk_t* chunk = &store->chunks[i];
    *chunk = (typeof(*chunk)) {0};
    loworld_chunk_initialize(&chunk->data);
  }
  return store;
}

void loworld_store_delete(loworld_store_t* store) {
  if (store == NULL) return;

  msgpack_unpacker_destroy(&store->unpacker);
  msgpack_unpacked_destroy(&store->unpacked);

  for (size_t i = 0; i < store->chunks_length; ++i) {
    loworld_chunk_deinitialize(&store->chunks[i].data);
  }
  memory_delete(store);
}

loworld_chunk_t* loworld_store_load_chunk(
    loworld_store_t* store, int32_t chunk_x, int32_t chunk_y) {
  assert(store != NULL);

  size_t index;
  if (!loworld_store_find_chunk_index_(store, &index, chunk_x, chunk_y)) {
    if (!loworld_store_find_unused_chunk_index_(store, &index)) {
      fprintf(stderr, "world store chunk overflow\n");
      abort();
    }
  }

  loworld_store_chunk_t* chunk = &store->chunks[index];

  if (chunk->loaded &&
      (chunk->data.pos.x != chunk_x || chunk->data.pos.y != chunk_y)) {
    assert(!chunk->used);
    store->error = !loworld_store_save_chunk_to_file_(store, &chunk->data);
    chunk->loaded = false;
  }
  if (!chunk->loaded) {
    chunk->data.pos.x = chunk_x;
    chunk->data.pos.y = chunk_y;

    if (!loworld_store_load_chunk_from_file_(store, &chunk->data)) {
      loworld_chunk_clear(&chunk->data);
      loworld_generator_generate(store->generator, &chunk->data);
    }
    chunk->loaded = true;
  }

  chunk->used       = true;
  chunk->last_tick  = store->tick++;
  return &chunk->data;
}

void loworld_store_unload_chunk(
    loworld_store_t* store, loworld_chunk_t* chunk) {
  assert(store != NULL);
  assert(chunk != NULL);

  loworld_store_chunk_t* c = (typeof(c)) chunk;
  assert(store->chunks <= c && c < store->chunks+store->chunks_length);

  c->used = false;
}

void loworld_store_flush(loworld_store_t* store) {
  assert(store != NULL);

  for (size_t i = 0; i < store->chunks_length; ++i) {
    loworld_store_chunk_t* chunk = &store->chunks[i];
    if (!chunk->loaded) continue;

    store->error = !loworld_store_save_chunk_to_file_(store, &chunk->data);
  }
}

bool loworld_store_is_error_happened(const loworld_store_t* store) {
  assert(store != NULL);

  return store->error;
}
