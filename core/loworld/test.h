#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <msgpack.h>
#include <msgpack/sbuffer.h>

#include "core/lobullet/base.h"
#include "core/lobullet/bomb.h"
#include "core/lobullet/linear.h"
#include "core/locharacter/base.h"
#include "core/locharacter/big_warder.h"
#include "core/locharacter/cavia.h"
#include "core/locharacter/encephalon.h"
#include "core/locharacter/greedy_scientist.h"
#include "core/locharacter/theists_child.h"
#include "core/locharacter/scientist.h"
#include "core/locharacter/warder.h"
#include "core/locharacter/pool.h"
#include "core/loentity/entity.h"
#include "core/loground/base.h"
#include "core/loground/island.h"
#include "core/loground/pool.h"

#include "./poolset.h"

# define pack_and_unpack_(type, name) do {  \
    msgpack_sbuffer_clear(buf);  \
    loentity_pack(&base->super.super, &packer);  \
    loentity_delete(&base->super.super);  \
  \
    size_t offset = 0;  \
    const msgpack_unpack_return r =  \
        msgpack_unpack_next(upk, buf->data, buf->size, &offset);  \
    if (r != MSGPACK_UNPACK_SUCCESS) {  \
      fprintf(stderr, #type"_"#name": invalid msgpack format\n");  \
      abort();  \
    }  \
    if (!type##_base_unpack(base, &upk->data)) {  \
      fprintf(stderr, #type"_"#name": failed to unpack\n");  \
      abort();  \
    }  \
    loentity_delete(&base->super.super);  \
    printf(#type"_"#name": pack test passed\n");  \
  } while (0)

static inline void loworld_test_packing_grounds(
    loground_base_t* base, msgpack_sbuffer* buf, msgpack_unpacked* upk) {
  assert(base != NULL);
  assert(buf  != NULL);
  assert(upk  != NULL);

  msgpack_packer packer;
  msgpack_packer_init(&packer, buf, msgpack_sbuffer_write);

  loground_island_build(
      base, &locommon_position(0, 0, vec2(0, 0)), &vec2(1, 1));
  pack_and_unpack_(loground, island);
}

static inline void loworld_test_packing_bullets(
    lobullet_base_t* base, msgpack_sbuffer* buf, msgpack_unpacked* upk) {
  assert(base != NULL);
  assert(buf  != NULL);
  assert(upk  != NULL);

  msgpack_packer packer;
  msgpack_packer_init(&packer, buf, msgpack_sbuffer_write);

  lobullet_bomb_square_build(
      base, &((lobullet_bomb_param_t) { .step = 1, }));
  pack_and_unpack_(lobullet, bomb);

  lobullet_bomb_triangle_build(
      base, &((lobullet_bomb_param_t) { .step = 1, }));
  pack_and_unpack_(lobullet, bomb);

  lobullet_linear_light_build(
      base, &((lobullet_linear_param_t) { .duration = 1, }));
  pack_and_unpack_(lobullet, linear);

  lobullet_linear_triangle_build(
      base, &((lobullet_linear_param_t) { .duration = 1, }));
  pack_and_unpack_(lobullet, linear);
}

static inline void loworld_test_packing_characters(
    locharacter_base_t* base, msgpack_sbuffer* buf, msgpack_unpacked* upk) {
  assert(base != NULL);
  assert(buf  != NULL);
  assert(upk  != NULL);

  msgpack_packer packer;
  msgpack_packer_init(&packer, buf, msgpack_sbuffer_write);

  locharacter_big_warder_build(base, 0);
  pack_and_unpack_(locharacter, big_warder);

  locharacter_cavia_build(
      base, &((locharacter_cavia_param_t) { .direction = 1, }));
  pack_and_unpack_(locharacter, cavia);

  locharacter_encephalon_build(base, 0);
  pack_and_unpack_(locharacter, encephalon);

  locharacter_theists_child_build(base, 0);
  pack_and_unpack_(locharacter, theists_child);

  locharacter_greedy_scientist_build(base, 0);
  pack_and_unpack_(locharacter, greedy_scientist);

  locharacter_scientist_build(base, &((locharacter_scientist_param_t) {0}));
  pack_and_unpack_(locharacter, scientist);

  locharacter_warder_build(base, &((locharacter_warder_param_t) {0}));
  pack_and_unpack_(locharacter, warder);
}

# undef pack_and_unpack_

static inline void loworld_test_packing(const loworld_poolset_t* pools) {
  assert(pools != NULL);

  msgpack_sbuffer buf;
  msgpack_sbuffer_init(&buf);

  msgpack_unpacked upk;
  msgpack_unpacked_init(&upk);

  loground_base_t* ground = loground_pool_create(pools->ground);
  loworld_test_packing_grounds(ground, &buf, &upk);
  loentity_delete(&ground->super.super);

  lobullet_base_t* bullet = lobullet_pool_create(pools->bullet);
  loworld_test_packing_bullets(bullet, &buf, &upk);
  loentity_delete(&bullet->super.super);

  locharacter_base_t* chara = locharacter_pool_create(pools->character);
  loworld_test_packing_characters(chara, &buf, &upk);
  loentity_delete(&chara->super.super);

  msgpack_unpacked_destroy(&upk);
  msgpack_sbuffer_destroy(&buf);
}
