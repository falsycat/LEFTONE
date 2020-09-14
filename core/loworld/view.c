#include "./view.h"

#include <assert.h>
#include <stddef.h>

#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "core/locommon/position.h"
#include "core/loentity/store.h"

#include "./chunk.h"
#include "./store.h"

#define LOWORLD_VIEW_CHUNK_LOAD_RANGE 2

struct loworld_view_t {
  loworld_store_t*  world;
  loentity_store_t* entities;

  locommon_position_t looking;

  loworld_chunk_t* chunks
      [LOWORLD_VIEW_CHUNK_LOAD_RANGE*2+1][LOWORLD_VIEW_CHUNK_LOAD_RANGE*2+1];
};

static void loworld_view_stage_chunk_(
    loworld_view_t* view, loworld_chunk_t* chunk) {
  assert(view  != NULL);
  assert(chunk != NULL);

  const size_t len = container_array_get_length(chunk->entities);
  for (size_t i = 0; i < len; ++i) {
    /* moving ownership from the chunk to the store */
    loentity_store_add(view->entities, chunk->entities[i]);
  }
  container_array_resize(chunk->entities, 0);
}

static void loworld_view_unstage_chunk_(
    loworld_view_t* view, loworld_chunk_t* chunk) {
  assert(view  != NULL);
  assert(chunk != NULL);

  loentity_store_iterator_t itr = {0};
  while (loentity_store_iterate_next(view->entities, &itr)) {
    if (itr.entity->dont_save ||
        itr.entity->pos.chunk.x != chunk->pos.x ||
        itr.entity->pos.chunk.y != chunk->pos.y) {
      continue;
    }

    const size_t index = container_array_get_length(chunk->entities);
    container_array_insert(chunk->entities, index);

    /* moving ownership from the store to the chunk */
    chunk->entities[index] = loentity_store_remove(view->entities, &itr);
  }
}

static void loworld_view_load_all_chunks_(loworld_view_t* view) {
  assert(view != NULL);

  /* Unload all chunks before calling this function. */

  static const int32_t r = LOWORLD_VIEW_CHUNK_LOAD_RANGE;

  for (int32_t x = -r; x <= r; ++x) {
    for (int32_t y = -r; y <= r; ++y) {
      loworld_chunk_t* chunk = loworld_store_load_chunk(
          view->world, view->looking.chunk.x + x, view->looking.chunk.y + y);
      view->chunks[y+r][x+r] = chunk;
      loworld_view_stage_chunk_(view, chunk);
    }
  }
}

static void loworld_view_unload_all_chunks_(loworld_view_t* view) {
  assert(view != NULL);

  static const int32_t r = LOWORLD_VIEW_CHUNK_LOAD_RANGE;

  for (int32_t x = -r; x <= r; ++x) {
    for (int32_t y = -r; y <= r; ++y) {
      loworld_chunk_t* chunk = view->chunks[y+r][x+r];
      if (chunk == NULL) continue;

      loworld_view_unstage_chunk_(view, chunk);
      loworld_store_unload_chunk(view->world, chunk);
    }
  }
}

loworld_view_t* loworld_view_new(
    loworld_store_t*           world,
    loentity_store_t*          entities,
    const locommon_position_t* looking) {
  assert(world    != NULL);
  assert(entities != NULL);
  assert(locommon_position_valid(looking));

  loworld_view_t* view = memory_new(sizeof(*view));
  *view = (typeof(*view)) {
    .world    = world,
    .entities = entities,
    .looking  = *looking,
  };

  loworld_view_load_all_chunks_(view);
  return view;
}

void loworld_view_delete(loworld_view_t* view) {
  if (view == NULL) return;

  loworld_view_unload_all_chunks_(view);

  memory_delete(view);
}

void loworld_view_update(loworld_view_t* view) {
  assert(view != NULL);

  loentity_store_iterator_t itr = {0};
  while (loentity_store_iterate_next(view->entities, &itr)) {
    if (!loentity_update(itr.entity)) {
      loentity_delete(loentity_store_remove(view->entities, &itr));
    }
  }
}

void loworld_view_draw(loworld_view_t* view) {
  assert(view != NULL);

  loentity_store_iterator_t itr = {0};
  while (loentity_store_iterate_next(view->entities, &itr)) {
    loentity_draw(itr.entity, &view->looking);
  }
}

void loworld_view_look(loworld_view_t* view, const locommon_position_t* pos) {
  assert(view != NULL);
  assert(locommon_position_valid(pos));

  const bool chunk_moved =
      view->looking.chunk.x != pos->chunk.x ||
      view->looking.chunk.y != pos->chunk.y;

  view->looking = *pos;

  if (chunk_moved) {
    loworld_view_unload_all_chunks_(view);
    loworld_view_load_all_chunks_(view);
  }
}

void loworld_view_flush_store(loworld_view_t* view) {
  assert(view != NULL);

  loworld_view_unload_all_chunks_(view);
  loworld_store_flush(view->world);
  loworld_view_load_all_chunks_(view);
}

const loworld_chunk_t* loworld_view_get_looking_chunk(
    const loworld_view_t* view) {
  assert(view != NULL);

  static const int32_t r = LOWORLD_VIEW_CHUNK_LOAD_RANGE;
  return view->chunks[r][r];
}
