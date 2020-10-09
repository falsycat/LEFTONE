#include "./store.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/container/array.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "core/locommon/physics.h"
#include "core/locommon/position.h"

#include "./bullet.h"
#include "./character.h"
#include "./entity.h"
#include "./ground.h"

struct loentity_store_t {
  CONTAINER_ARRAY loentity_t** items;
};

static void loentity_store_iterator_assign_by_index_(
    loentity_store_t* store, loentity_store_iterator_t* itr) {
  assert(store != NULL);
  assert(itr   != NULL);
  assert(itr->index < container_array_get_length(store->items));

  itr->entity    = NULL;
  itr->ground    = NULL;
  itr->bullet    = NULL;
  itr->character = NULL;

  itr->entity = store->items[itr->index];
  switch (itr->entity->subclass) {
  case LOENTITY_SUBCLASS_GROUND:
    itr->ground = (loentity_ground_t*) itr->entity;
    break;
  case LOENTITY_SUBCLASS_PARTICLE:
    break;
  case LOENTITY_SUBCLASS_BULLET:
    itr->bullet = (loentity_bullet_t*) itr->entity;
    break;
  case LOENTITY_SUBCLASS_CHARACTER:
    itr->character = (loentity_character_t*) itr->entity;
    break;
  default:
    assert(false);
  }
}

loentity_store_t* loentity_store_new(size_t reserve) {
  loentity_store_t* store = memory_new(sizeof(*store));
  *store = (typeof(*store)) {0};

  container_array_reserve(store->items, reserve);
  return store;
}

void loentity_store_delete(loentity_store_t* store) {
  if (store == NULL) return;

  container_array_delete(store->items);
  memory_delete(store);
}

void loentity_store_add(
    loentity_store_t* store, loentity_t* entity) {
  assert(store  != NULL);
  assert(entity != NULL);

  const size_t len = container_array_get_length(store->items);
  size_t index = 0;
  for (; index < len; ++index) {
    if (store->items[index] == NULL) break;
  }
  if (index == len) container_array_insert(store->items, index);
  store->items[index] = entity;
}

loentity_t* loentity_store_remove(
    loentity_store_t* store, const loentity_store_iterator_t* itr) {
  assert(store != NULL);
  assert(itr   != NULL);

  assert(itr->index < container_array_get_length(store->items));
  assert(itr->entity != NULL);
  assert(itr->entity == store->items[itr->index]);

  store->items[itr->index] = NULL;
  return itr->entity;
}

void loentity_store_clear(loentity_store_t* store) {
  assert(store != NULL);

  const size_t len = container_array_get_length(store->items);
  for (size_t i = 0; i < len; ++i) {
    loentity_t** e = &store->items[i];
    if (*e == NULL) continue;
    loentity_delete(*e);
    *e = NULL;
  }
}

bool loentity_store_iterate_next(
    loentity_store_t* store, loentity_store_iterator_t* itr) {
  assert(store != NULL);
  assert(itr   != NULL);

  ++itr->index;
  if (itr->entity == NULL) {
    itr->index = 0;
  }
  itr->entity    = NULL;
  itr->ground    = NULL;
  itr->bullet    = NULL;
  itr->character = NULL;

  const size_t len = container_array_get_length(store->items);
  for (; itr->index < len; ++itr->index) {
    if (store->items[itr->index] != NULL) break;
  }
  if (itr->index >= len) return false;

  loentity_store_iterator_assign_by_index_(store, itr);
  return true;
}

bool loentity_store_find_item_by_id(
    loentity_store_t* store, loentity_store_iterator_t* itr, loentity_id_t id) {
  assert(store != NULL);
  assert(itr   != NULL);

  const size_t len = container_array_get_length(store->items);
  for (itr->index = 0; itr->index < len; ++itr->index) {
    loentity_t* e = store->items[itr->index];
    if (e != NULL && e->id == id) {
      loentity_store_iterator_assign_by_index_(store, itr);
      return true;
    }
  }
  return false;
}

bool loentity_store_solve_collision_between_ground(
    loentity_store_t*          store,
    locommon_physics_entity_t* e,
    float                      dt) {
  assert(store != NULL);
  assert(e     != NULL);

  bool solved = false;

  loentity_store_iterator_t itr = {0};
  while (loentity_store_iterate_next(store, &itr)) {
    if (itr.ground == NULL) continue;

    const locommon_physics_entity_t g = {
      .pos      = itr.entity->pos,
      .velocity = vec2(0, 0),
      .size     = itr.ground->size,
    };
    if (locommon_physics_solve_collision_with_fixed_one(e, &g, dt)) {
      solved = true;
    }
  }
  return solved;
}

bool loentity_store_affect_bullets_shot_by_others(
    loentity_store_t*     store,
    loentity_character_t* chara,
    const vec2_t*         velocity,
    float                 dt) {
  assert(store != NULL);
  assert(chara != NULL);
  assert(vec2_valid(velocity));
  assert(MATH_FLOAT_VALID(dt));

  loentity_store_iterator_t itr = {0};
  while (loentity_store_iterate_next(store, &itr)) {
    if (itr.bullet == NULL || itr.bullet->owner == chara->super.id) continue;

    if (loentity_bullet_hittest(itr.bullet, &chara->super.pos, velocity, dt)) {
      if (loentity_bullet_affect(itr.bullet, chara)) return true;
    }
  }
  return false;
}

bool loentity_store_affect_bullets_shot_by_one(
    loentity_store_t*     store,
    loentity_character_t* chara,
    loentity_id_t         shooter,
    const vec2_t*         velocity,
    float                 dt) {
  assert(store != NULL);
  assert(chara != NULL);
  assert(vec2_valid(velocity));
  assert(MATH_FLOAT_VALID(dt));

  loentity_store_iterator_t itr = {0};
  while (loentity_store_iterate_next(store, &itr)) {
    if (itr.bullet == NULL || itr.bullet->owner != shooter) continue;

    if (loentity_bullet_hittest(itr.bullet, &chara->super.pos, velocity, dt)) {
      if (loentity_bullet_affect(itr.bullet, chara)) return true;
    }
  }
  return false;
}
