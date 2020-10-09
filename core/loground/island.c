#include <stdbool.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/locommon/position.h"

#include "./base.h"
#include "./type.h"

bool loground_island_update(loground_base_t* base) {
  assert(base != NULL);

  base->cache.instance = (loshader_ground_drawer_instance_t) {
    .ground_id = LOSHADER_GROUND_ID_ISLAND,
    .size      = base->super.size,
  };
  return true;
}

void loground_island_build(
    loground_base_t*           base,
    const locommon_position_t* pos,
    const vec2_t*              size) {
  assert(base != NULL);
  assert(locommon_position_valid(pos));
  assert(vec2_valid(size));
  assert(size->x >= 0 && size->y >= 0);

  base->param = (typeof(base->param)) {
    .type = LOGROUND_TYPE_ISLAND,
  };

  base->super.super.pos = *pos;
  base->super.size      = *size;
}
