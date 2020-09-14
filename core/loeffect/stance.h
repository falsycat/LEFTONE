#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "core/loshader/menu_stance.h"

typedef enum {
  LOEFFECT_STANCE_ID_MISSIONARY,
  LOEFFECT_STANCE_ID_REVOLUTIONER,
  LOEFFECT_STANCE_ID_UNFINISHER,
  LOEFFECT_STANCE_ID_PHILOSOPHER,
  LOEFFECT_STANCE_ID_LENGTH_,
} loeffect_stance_id_t;
_Static_assert(LOEFFECT_STANCE_ID_LENGTH_ < 16);

typedef uint16_t loeffect_stance_set_t;

#define LOEFFECT_STANCE_EACH(PROC) do {  \
  PROC(MISSIONARY,   missionary);  \
  PROC(REVOLUTIONER, revolutioner);  \
  PROC(UNFINISHER,   unfinisher);  \
  PROC(PHILOSOPHER,  philosopher);  \
} while (0)

const char*
loeffect_stance_stringify(
    loeffect_stance_id_t id
);

bool
loeffect_stance_unstringify(
    loeffect_stance_id_t* id,
    const char*           str,
    size_t                len
);

loshader_menu_stance_id_t
loeffect_stance_get_id_for_menu_shader(
    loeffect_stance_id_t id
);

void
loeffect_stance_set_initialize(
    loeffect_stance_set_t* set
);

void
loeffect_stance_set_deinitialize(
    loeffect_stance_set_t* set
);

void
loeffect_stance_set_add(
    loeffect_stance_set_t* set,
    loeffect_stance_id_t   id
);

void
loeffect_stance_set_remove(
    loeffect_stance_set_t* set,
    loeffect_stance_id_t   id
);

bool
loeffect_stance_set_has(
    const loeffect_stance_set_t* set,
    loeffect_stance_id_t         id
);

void
loeffect_stance_set_pack(
    const loeffect_stance_set_t* set,
    msgpack_packer*              packer
);

bool
loeffect_stance_set_unpack(
    loeffect_stance_set_t* set,
    const msgpack_object*  obj
);
