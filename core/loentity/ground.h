#pragma once

#include "util/math/vector.h"

#include "./entity.h"

#include "./decl.private.h"

typedef struct {
} loentity_ground_vtable_t;

struct loentity_ground_t {
  loentity_t super;

  loentity_ground_vtable_t vtable;

  vec2_t size;
};
