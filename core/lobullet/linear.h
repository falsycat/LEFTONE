#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/loeffect/effect.h"
#include "core/loentity/entity.h"

#include "./base.h"

typedef struct {
  loentity_id_t owner;

  locommon_position_t pos;
  vec2_t              size;
  vec2_t              velocity;
  vec2_t              acceleration;
  vec4_t              color;

  uint64_t duration;
  float    knockback;

  loeffect_t effect;

  uint64_t since;  /* set by build function */
} lobullet_linear_param_t;

bool
lobullet_linear_param_valid(
    const lobullet_linear_param_t* param
);
void
lobullet_linear_param_pack(
    const lobullet_linear_param_t* param,
    msgpack_packer*                packer
);
bool
lobullet_linear_param_unpack(
    lobullet_linear_param_t* param,
    const msgpack_object*    obj  /* NULLABLE */
);

void
lobullet_linear_build(
    lobullet_base_t*               base,
    lobullet_type_t                type,
    const lobullet_linear_param_t* param
);

bool
lobullet_linear_light_update(
    lobullet_base_t* base
);
#define lobullet_linear_light_build(base, param)  \
    lobullet_linear_build(base, LOBULLET_TYPE_LINEAR_LIGHT, param)
#define lobullet_linear_light_tear_down(base)
#define lobullet_linear_light_pack_data(base, packer)  \
    lobullet_linear_param_pack(  \
        (const lobullet_linear_param_t*) base->data, packer)
#define lobullet_linear_light_unpack_data(base, obj)  \
    lobullet_linear_param_unpack(  \
        (lobullet_linear_param_t*) base->data, obj)

bool
lobullet_linear_triangle_update(
    lobullet_base_t* base
);
#define lobullet_linear_triangle_build(base, param)  \
    lobullet_linear_build(base, LOBULLET_TYPE_LINEAR_TRIANGLE, param)
#define lobullet_linear_triangle_tear_down(base)
#define lobullet_linear_triangle_pack_data(base, packer)  \
    lobullet_linear_param_pack(  \
        (const lobullet_linear_param_t*) base->data, packer)
#define lobullet_linear_triangle_unpack_data(base, obj)  \
    lobullet_linear_param_unpack(  \
        (lobullet_linear_param_t*) base->data, obj)
