#include "./character.h"

#include <assert.h>
#include <stddef.h>

#include "core/loeffect/effect.h"

#include "./entity.h"

#include "./decl.private.h"

void loentity_character_apply_effect(
    loentity_character_t* chara, const loeffect_t* effect) {
  assert(chara  != NULL);
  assert(effect != NULL);

  assert(chara->vtable.apply_effect != NULL);
  chara->vtable.apply_effect(chara, effect);
}

void loentity_character_knockback(
    loentity_character_t* chara, const vec2_t* v) {
  assert(chara != NULL);
  assert(v     != NULL);

  assert(chara->vtable.knockback != NULL);
  chara->vtable.knockback(chara, v);
}
