#pragma once

#include <stdbool.h>

#include <msgpack.h>

#include "core/locommon/ticker.h"
#include "core/loeffect/stance.h"
#include "core/loresource/set.h"
#include "core/loshader/set.h"

#include "./controller.h"
#include "./entity.h"
#include "./status.h"

struct loplayer_menu_t;
typedef struct loplayer_menu_t loplayer_menu_t;

loplayer_menu_t*  /* OWNERSHIP */
loplayer_menu_new(
    loresource_set_t*            res,
    loshader_set_t*              shaders,
    const locommon_ticker_t*     ticker,
    const loplayer_status_t*     status,
    const loplayer_controller_t* controller
);

void
loplayer_menu_delete(
    loplayer_menu_t* menu  /* OWNERSHIP */
);

void
loplayer_menu_show_status(
    loplayer_menu_t* menu
);

void
loplayer_menu_show_status_with_stance_highlighted(
    loplayer_menu_t*     menu,
    loeffect_stance_id_t id
);

void
loplayer_menu_popup(
    loplayer_menu_t* menu,
    const char*      title,
    const char*      text
);

void
loplayer_menu_hide(
    loplayer_menu_t* menu
);

void
loplayer_menu_update(
    loplayer_menu_t* menu
);

void
loplayer_menu_draw_ui(
    const loplayer_menu_t* menu
);

bool
loplayer_menu_is_shown(
    const loplayer_menu_t* menu
);

bool
loplayer_menu_is_exit_requested(
    const loplayer_menu_t* menu
);

void
loplayer_menu_pack(
    const loplayer_menu_t* menu,
    msgpack_packer*        packer
);

bool
loplayer_menu_unpack(
    loplayer_menu_t*      menu,
    const msgpack_object* obj  /* NULLABLE */
);
