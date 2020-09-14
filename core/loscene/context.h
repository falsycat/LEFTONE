#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "util/math/vector.h"

#include "core/locommon/input.h"

#include "./param.h"

struct loscene_context_t;
typedef struct loscene_context_t loscene_context_t;

loscene_context_t*  /* OWNERSHIP */
loscene_context_new(
    const loscene_param_t* param
);

void
loscene_context_delete(
    loscene_context_t* ctx  /* OWNERSHIP */
);

bool
loscene_context_update(
    loscene_context_t*       ctx,
    const locommon_input_t*  input,
    uint64_t                 uptime
);

void
loscene_context_draw(
    loscene_context_t* ctx
);
