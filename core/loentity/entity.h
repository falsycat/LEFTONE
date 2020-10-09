#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/locommon/position.h"

#include "./decl.private.h"

typedef struct {
  void
  (*delete)(
      loentity_t* entity
  );

  void
  (*die)(
      loentity_t* entity
  );

  bool
  (*update)(
      loentity_t* entity
  );
  void
  (*draw)(
      loentity_t*                entity,
      const locommon_position_t* basepos
  );

  void
  (*pack)(
      const loentity_t* entity,
      msgpack_packer*   packer
  );
} loentity_vtable_t;

typedef enum {
  LOENTITY_SUBCLASS_NONE,
  LOENTITY_SUBCLASS_GROUND,
  LOENTITY_SUBCLASS_PARTICLE,
  LOENTITY_SUBCLASS_BULLET,
  LOENTITY_SUBCLASS_CHARACTER,
} loentity_subclass_t;

typedef uint64_t loentity_id_t;

struct loentity_t {
  loentity_vtable_t   vtable;
  loentity_subclass_t subclass;

  loentity_id_t id;

  locommon_position_t pos;

  bool dont_save;
};

void
loentity_delete(
    loentity_t* entity  /* OWNERSHIP */
);

void
loentity_die(
    loentity_t* entity
);

bool
loentity_update(
    loentity_t* entity
);

void
loentity_draw(
    loentity_t*                entity,
    const locommon_position_t* basepos
);

void
loentity_pack(
    const loentity_t* entity,
    msgpack_packer*   packer
);
