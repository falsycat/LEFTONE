#include "./chunk.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <msgpack.h>

#include "util/container/array.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/loentity/entity.h"

#include "./poolset.h"

static void loworld_chunk_pack_entities_(
    const loworld_chunk_t* chunk, msgpack_packer* packer) {
  assert(chunk  != NULL);
  assert(packer != NULL);

  const size_t len = container_array_get_length(chunk->entities);
  msgpack_pack_array(packer, len);
  for (size_t i = 0; i < len; ++i) {
    loentity_pack(chunk->entities[i], packer);
  }
}

const char* loworld_chunk_biome_stringify(loworld_chunk_biome_t biome) {
# define each_(NAME, name)  \
      if (biome == LOWORLD_CHUNK_BIOME_##NAME) return #name;

  LOWORLD_CHUNK_BIOME_EACH_(each_);

  assert(false);
  return NULL;

# undef each_
}

bool loworld_chunk_biome_unstringify(
    loworld_chunk_biome_t* biome, const char* str, size_t len) {
  assert(biome != NULL);
  assert(str != NULL || len == 0);

# define each_(NAME, name) do {  \
    if (strncmp(str, #name, len) == 0 && #name[len] == 0) {  \
      *biome = LOWORLD_CHUNK_BIOME_##NAME;  \
      return true;  \
    }  \
  } while (0)

  LOWORLD_CHUNK_BIOME_EACH_(each_);
  return false;

# undef each_
}

void loworld_chunk_initialize(loworld_chunk_t* chunk) {
  assert(chunk != NULL);

  *chunk = (typeof(*chunk)) {0};
}

void loworld_chunk_deinitialize(loworld_chunk_t* chunk) {
  if (chunk == NULL) return;

  loworld_chunk_clear(chunk);
  container_array_delete(chunk->entities);
}

void loworld_chunk_add_entity(loworld_chunk_t* chunk, loentity_t* entity) {
  assert(chunk  != NULL);
  assert(entity != NULL);

  const size_t index = container_array_get_length(chunk->entities);
  container_array_insert(chunk->entities, index);
  chunk->entities[index] = entity;
}

void loworld_chunk_clear(loworld_chunk_t* chunk) {
  assert(chunk != NULL);

  const size_t len = container_array_get_length(chunk->entities);
  for (size_t i = 0; i < len; ++i) {
    loentity_delete(chunk->entities[i]);
  }
  container_array_resize(chunk->entities, 0);
}

void loworld_chunk_pack(const loworld_chunk_t* chunk, msgpack_packer* packer) {
  assert(chunk  != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 3);

  mpkutil_pack_str(packer, "pos");
  msgpack_pack_array(packer, 2);
  msgpack_pack_int32(packer, chunk->pos.x);
  msgpack_pack_int32(packer, chunk->pos.y);

  mpkutil_pack_str(packer, "biome");
  mpkutil_pack_str(packer, loworld_chunk_biome_stringify(chunk->biome));

  mpkutil_pack_str(packer, "entities");
  loworld_chunk_pack_entities_(chunk, packer);
}

bool loworld_chunk_unpack(
    loworld_chunk_t*         chunk,
    const msgpack_object*    obj,
    const loworld_poolset_t* pools) {
  assert(chunk != NULL);
  assert(obj   != NULL);
  assert(pools != NULL);

  const msgpack_object_map* root = mpkutil_get_map(obj);
  if (root == NULL) return false;

#define item_(name) mpkutil_get_map_item_by_str(root, name)

  const msgpack_object_array* pos = mpkutil_get_array(item_("pos"));
  if (pos == NULL || pos->size != 2 ||
      !mpkutil_get_int32(&pos->ptr[0], &chunk->pos.x) ||
      !mpkutil_get_int32(&pos->ptr[1], &chunk->pos.y)) {
    return false;
  }

  const char* biome;
  size_t      biome_len;
  if (!mpkutil_get_str(item_("biome"), &biome, &biome_len) ||
      !loworld_chunk_biome_unstringify(&chunk->biome, biome, biome_len)) {
    return false;
  }

  const msgpack_object_array* entities = mpkutil_get_array(item_("entities"));
  if (entities != NULL) {
    container_array_reserve(chunk->entities, entities->size);
    for (size_t i = 0; i < entities->size; ++i) {
      loentity_t* e = loworld_poolset_unpack_entity(pools, &entities->ptr[i]);
      if (e != NULL) loworld_chunk_add_entity(chunk, e);
    }
  }

#undef item_
  return true;
}

void loworld_chunk_build_filename(
    const loworld_chunk_t* chunk, char* filename, size_t length) {
  assert(chunk != NULL);
  assert(filename != NULL || length == 0);

  snprintf(filename, length,
      "%"PRId32"_%"PRId32".msgpack", chunk->pos.x, chunk->pos.y);
}
