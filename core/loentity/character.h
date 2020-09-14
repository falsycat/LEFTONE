#pragma once

#include "util/math/vector.h"

#include "core/loeffect/effect.h"

#include "./entity.h"

#include "./decl.private.h"

typedef struct {
  void
  (*apply_effect)(
      loentity_character_t* chara,
      const loeffect_t*     effect
  );
  void
  (*knockback)(
      loentity_character_t* chara,
      const vec2_t*         v
  );
} loentity_character_vtable_t;

struct loentity_character_t {
  loentity_t super;

  loentity_character_vtable_t vtable;
};

void
loentity_character_apply_effect(
    loentity_character_t* chara,
    const loeffect_t*     effect
);

void
loentity_character_knockback(
    loentity_character_t* chara,
    const vec2_t*         v
);
