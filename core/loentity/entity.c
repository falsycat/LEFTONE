#include "./entity.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "./decl.private.h"

void loentity_delete(loentity_t* entity) {
  assert(entity != NULL);

  assert(entity->vtable.delete != NULL);
  entity->vtable.delete(entity);
}

void loentity_die(loentity_t* entity) {
  assert(entity != NULL);

  assert(entity->vtable.die != NULL);
  entity->vtable.die(entity);
}

bool loentity_update(loentity_t* entity) {
  assert(entity != NULL);

  assert(entity->vtable.update != NULL);
  return entity->vtable.update(entity);
}

void loentity_draw(loentity_t* entity, const locommon_position_t* basepos) {
  assert(entity != NULL);
  assert(locommon_position_valid(basepos));

  assert(entity->vtable.draw != NULL);
  entity->vtable.draw(entity, basepos);
}

void loentity_pack(const loentity_t* entity, msgpack_packer* packer) {
  assert(entity != NULL);
  assert(packer != NULL);

  assert(entity->vtable.pack != NULL);
  entity->vtable.pack(entity, packer);
}
