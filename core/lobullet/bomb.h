#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/loeffect/effect.h"
#include "core/loentity/entity.h"

#include "./base.h"
#include "./misc.h"

typedef struct {
  loentity_id_t       owner;
  locommon_position_t pos;
  vec2_t              size;
  float               angle;
  vec4_t              color;
  bool                silent;

  float   beat;
  int32_t step;
  float   knockback;

  loeffect_t effect;

  uint64_t since;  /* set by build function */
} lobullet_bomb_param_t;

bool
lobullet_bomb_param_valid(
    const lobullet_bomb_param_t* param  /* NULLABLE */
);
void
lobullet_bomb_param_pack(
    const lobullet_bomb_param_t* param,
    msgpack_packer*              packer
);
bool
lobullet_bomb_param_unpack(
    lobullet_bomb_param_t* param,
    const msgpack_object*  obj  /* NULLABLE */
);

void
lobullet_bomb_build(
    lobullet_base_t*             base,
    lobullet_type_t              type,
    const lobullet_bomb_param_t* param
);

bool
lobullet_bomb_square_update(
    lobullet_base_t* base
);
#define lobullet_bomb_square_build(base, param)  \
    lobullet_bomb_build(base, LOBULLET_TYPE_BOMB_SQUARE, param)
#define lobullet_bomb_square_tear_down(base)
#define lobullet_bomb_square_pack_data(base, packer)  \
    lobullet_bomb_param_pack(  \
        (const lobullet_bomb_param_t*) base->data, packer)
#define lobullet_bomb_square_unpack_data(base, obj)  \
    lobullet_bomb_param_unpack(  \
        (lobullet_bomb_param_t*) base->data, obj)

bool
lobullet_bomb_triangle_update(
    lobullet_base_t* base
);
#define lobullet_bomb_triangle_build(base, param)  \
    lobullet_bomb_build(base, LOBULLET_TYPE_BOMB_TRIANGLE, param)
#define lobullet_bomb_triangle_tear_down(base)
#define lobullet_bomb_triangle_pack_data(base, packer)  \
    lobullet_bomb_param_pack(  \
        (const lobullet_bomb_param_t*) base->data, packer)
#define lobullet_bomb_triangle_unpack_data(base, obj)  \
    lobullet_bomb_param_unpack(  \
        (lobullet_bomb_param_t*) base->data, obj)
