#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "core/loshader/menu_stance.h"

typedef enum {
  /* BENUM BEGIN loplayer_stance */
  LOPLAYER_STANCE_MISSIONARY,
  LOPLAYER_STANCE_REVOLUTIONER,
  LOPLAYER_STANCE_UNFINISHER,
  LOPLAYER_STANCE_PHILOSOPHER,
  LOPLAYER_STANCE_BETRAYER,
  /* BENUM END */
} loplayer_stance_t;

#include "core/loplayer/benum/stance.h"
_Static_assert(LOPLAYER_STANCE_COUNT < 16, "too many stances");

typedef uint16_t loplayer_stance_set_t;

const char*
loplayer_stance_stringify(
    loplayer_stance_t stance
);

bool
loplayer_stance_unstringify(
    loplayer_stance_t* stance,
    const char*        str,
    size_t             len
);

loshader_menu_stance_id_t
loplayer_stance_get_menu_shader_id(
    loplayer_stance_t stance
);

void
loplayer_stance_set_initialize(
    loplayer_stance_set_t* set
);

void
loplayer_stance_set_deinitialize(
    loplayer_stance_set_t* set
);

void
loplayer_stance_set_add(
    loplayer_stance_set_t* set,
    loplayer_stance_t      stance
);

void
loplayer_stance_set_remove(
    loplayer_stance_set_t* set,
    loplayer_stance_t      stance
);

bool
loplayer_stance_set_has(
    const loplayer_stance_set_t* set,
    loplayer_stance_t            stance
);

void
loplayer_stance_set_pack(
    const loplayer_stance_set_t* set,
    msgpack_packer*              packer
);

bool
loplayer_stance_set_unpack(
    loplayer_stance_set_t* set,
    const msgpack_object*  obj
);
