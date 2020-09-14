#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "util/math/vector.h"

#include "core/locommon/physics.h"
#include "core/locommon/position.h"

#include "./bullet.h"
#include "./character.h"
#include "./entity.h"
#include "./ground.h"

struct loentity_store_t;
typedef struct loentity_store_t loentity_store_t;

typedef struct {
  loentity_t*           entity;
  loentity_ground_t*    ground;
  loentity_bullet_t*    bullet;
  loentity_character_t* character;

  size_t index;
} loentity_store_iterator_t;

loentity_store_t*  /* OWNERSHIP */
loentity_store_new(
    size_t reserve
);

void
loentity_store_delete(
    loentity_store_t* store  /* OWNERSHIP */
);

void
loentity_store_add(
    loentity_store_t* store,
    loentity_t*       entity  /* OWNERSHIP */
);

loentity_t*  /* OWNERSHIP */
loentity_store_remove(
    loentity_store_t*                store,
    const loentity_store_iterator_t* itr
);

void
loentity_store_clear(
    loentity_store_t* store
);

bool
loentity_store_iterate_next(
    loentity_store_t*          store,
    loentity_store_iterator_t* itr
);

bool
loentity_store_find_item_by_id(
    loentity_store_t*          store,
    loentity_store_iterator_t* itr,
    loentity_id_t              id
);

bool  /* whether the entitiy was collided */
loentity_store_solve_collision_between_ground(
    loentity_store_t*          store,
    locommon_physics_entity_t* e,
    float                      dt
);

bool
loentity_store_affect_bullets_shot_by_others(
    loentity_store_t*     store,
    loentity_character_t* chara,
    const vec2_t*         velocity,
    float                 dt
);

bool
loentity_store_affect_bullets_shot_by_one(
    loentity_store_t*     store,
    loentity_character_t* chara,
    loentity_id_t         shooter,
    const vec2_t*         velocity,
    float                 dt
);
