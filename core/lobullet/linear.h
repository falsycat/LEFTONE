#pragma once

#include <stdbool.h>

#include "util/math/vector.h"

#include "core/locommon/position.h"

#include "./base.h"

bool
lobullet_linear_circle_update(
    lobullet_base_t* base
);

bool
lobullet_linear_triangle_update(
    lobullet_base_t* base
);

bool
lobullet_linear_square_update(
    lobullet_base_t* base
);

void
lobullet_linear_build_(
    lobullet_base_t*             base,
    const lobullet_base_param_t* param
);
#define lobullet_linear_circle_build(base, ...)  \
    lobullet_linear_build_(  \
        base,  \
        &(lobullet_base_param_t) {  \
          .type = LOBULLET_TYPE_LINEAR_CIRCLE,  \
          __VA_ARGS__  \
        })
#define lobullet_linear_triangle_build(base, ...)  \
    lobullet_linear_build_(  \
        base,  \
        &(lobullet_base_param_t) {  \
          .type = LOBULLET_TYPE_LINEAR_TRIANGLE,  \
          __VA_ARGS__  \
        })
#define lobullet_linear_square_build(base, ...)  \
    lobullet_linear_build_(  \
        base,  \
        &(lobullet_base_param_t) {  \
          .type = LOBULLET_TYPE_LINEAR_SQUARE,  \
          __VA_ARGS__  \
        })
