#pragma once

#include <stdbool.h>

#include "util/coly2d/shape.h"
#include "util/math/vector.h"

#include "core/locommon/position.h"

#include "./entity.h"

#include "./decl.private.h"

typedef struct {
  bool
  (*affect)(
      loentity_bullet_t*    bullet,
      loentity_character_t* chara
  );
} loentity_bullet_vtable_t;

struct loentity_bullet_t {
  loentity_t super;

  loentity_bullet_vtable_t vtable;

  loentity_id_t owner;
  vec2_t        velocity;

  coly2d_shape_t shape;
};

bool
loentity_bullet_affect(
    loentity_bullet_t*    bullet,
    loentity_character_t* chara
);

bool
loentity_bullet_hittest(
    const loentity_bullet_t*   bullet,
    const locommon_position_t* point,
    const vec2_t*              velocity,
    float                      dt
);
