#pragma once

#include <stdbool.h>

#include "util/math/vector.h"

#include "./position.h"

typedef struct {
  /* input */
  vec2_t size;

  /* input/output */
  locommon_position_t pos;
  vec2_t              velocity;
} locommon_physics_entity_t;

bool
locommon_physics_entity_valid(
    const locommon_physics_entity_t* e
);

bool  /* whether they were collided */
locommon_physics_solve_collision_with_fixed_one(
    locommon_physics_entity_t*       e1,
    const locommon_physics_entity_t* e2,
    float                            dt
);
