#pragma once

#include "core/locommon/position.h"
#include "core/loentity/store.h"

#include "./chunk.h"
#include "./store.h"

struct loworld_view_t;
typedef struct loworld_view_t loworld_view_t;

loworld_view_t*  /* OWNERSHIP */
loworld_view_new(
    loworld_store_t*           world,
    loentity_store_t*          entities,
    const locommon_position_t* looking
);

void
loworld_view_delete(
    loworld_view_t* view  /* OWNERSHIP */
);

void
loworld_view_update(
    loworld_view_t* view
);

void
loworld_view_draw(
    loworld_view_t* view
);

void
loworld_view_look(
    loworld_view_t*            view,
    const locommon_position_t* pos
);

/* Flushes all chunks safely including currently staged ones. */
void
loworld_view_flush_store(
    loworld_view_t* view
);

const loworld_chunk_t*  /* ALIVES UNTIL NEXT OPERATION */
loworld_view_get_looking_chunk(
    const loworld_view_t* view
);
