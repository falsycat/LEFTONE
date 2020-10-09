#include "./template.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/chaos/xorshift.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "core/lochara/base.h"
#include "core/lochara/big_warder.h"
#include "core/lochara/cavia.h"
#include "core/lochara/encephalon.h"
#include "core/lochara/theists_child.h"
#include "core/lochara/warder.h"
#include "core/locommon/position.h"
#include "core/loentity/entity.h"
#include "core/loentity/ground.h"
#include "core/loground/base.h"
#include "core/loground/island.h"

#include "./chunk.h"
#include "./poolset.h"

static loentity_ground_t* loworld_template_add_ground_island_(
    const loworld_template_building_param_t* param,
    const vec2_t*                            pos,
    const vec2_t*                            sz) {
  assert(loworld_template_building_param_valid(param));
  assert(vec2_valid(pos));
  assert(vec2_valid(sz));
  assert(sz->x >= 0 && sz->y >= 0);

  const locommon_position_t p = locommon_position(
      param->target->pos.x, param->target->pos.y, *pos);

  loground_base_t* island = loground_pool_create(param->pools->ground);
  loground_island_build(island, &p, sz);
  loworld_chunk_add_entity(param->target, &island->super.super);
  return &island->super;
}

static void loworld_template_add_random_enemy_(
    const loworld_template_building_param_t* param,
    uint64_t                                 seed,
    const loentity_ground_t*                 gnd,
    float                                    pos) {
  assert(loworld_template_building_param_valid(param));
  assert(gnd != NULL);
  assert(MATH_FLOAT_VALID(pos));

  static void (*const builders[])(
      lochara_base_t*, const loentity_ground_t*, float) = {
    lochara_cavia_build,
    lochara_warder_build,
  };

  lochara_base_t* base = lochara_pool_create(param->pools->chara);

  const size_t i = chaos_xorshift(seed)%(sizeof(builders)/sizeof(builders[0]));
  builders[i](base, gnd, pos);

  loworld_chunk_add_entity(param->target, &base->super.super);
}

bool loworld_template_building_param_valid(
    const loworld_template_building_param_t* param) {
  return
      param != NULL &&
      param->target != NULL &&
      param->pools != NULL;
}

void loworld_template_metaphysical_gate_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  loworld_template_add_ground_island_(param, &vec2(.5f, .2f), &vec2(.5f, .1f));

  const loentity_ground_t* platform = loworld_template_add_ground_island_(
      param, &vec2(.5f, .45f), &vec2(.2f, .02f));

  lochara_base_t* encephalon = lochara_pool_create(param->pools->chara);
  lochara_encephalon_build(encephalon, platform);
  loworld_chunk_add_entity(param->target, &encephalon->super.super);
}

void loworld_template_open_space_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  uint64_t s = param->seed;

  const size_t enemy_count = (s = chaos_xorshift(s))%3+1;

  const loentity_ground_t* gnd = loworld_template_add_ground_island_(
      param, &vec2(.5f, .2f), &vec2(.47f, .01f));

  for (size_t i = 0; i < enemy_count; ++i) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), gnd, .1f+.8f/enemy_count*i);
  }
}

void loworld_template_broken_open_space_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  uint64_t s = param->seed;

  const loentity_ground_t* floor1 = loworld_template_add_ground_island_(
      param, &vec2(.2f, .2f), &vec2(.18f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), floor1, .5f);
  }

  const loentity_ground_t* floor2 = loworld_template_add_ground_island_(
      param, &vec2(.8f, .2f), &vec2(.18f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), floor2, .5f);
  }

  loworld_template_add_ground_island_(
      param, &vec2(.5f, .05f), &vec2(.15f, .01f));
}

void loworld_template_passage_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  uint64_t s = param->seed;

  const loentity_ground_t* floor = loworld_template_add_ground_island_(
      param, &vec2(.5f, .25f), &vec2(.5f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), floor, .5f);
  }

  const loentity_ground_t* ceiling = loworld_template_add_ground_island_(
      param, &vec2(.55f, .4f), &vec2(.3f, .007f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), ceiling, .5f);
  }
}

void loworld_template_broken_passage_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  uint64_t s = param->seed;

  const loentity_ground_t* floor1 = loworld_template_add_ground_island_(
      param, &vec2(.15f, .25f), &vec2(.15f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), floor1, .5f);
  }

  const loentity_ground_t* floor2 = loworld_template_add_ground_island_(
      param, &vec2(.45f, .25f), &vec2(.1f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), floor2, .5f);
  }

  const loentity_ground_t* floor3 = loworld_template_add_ground_island_(
      param, &vec2(.85f, .25f), &vec2(.15f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), floor3, .5f);
  }

  const uint64_t layout = (s = chaos_xorshift(s))%3;
  if (layout == 0 || layout == 1) {
    const loentity_ground_t* ceiling = loworld_template_add_ground_island_(
        param, &vec2(.2f, .4f), &vec2(.15f, .007f));
    if ((s = chaos_xorshift(s))%2) {
      loworld_template_add_random_enemy_(
          param, s = chaos_xorshift(s), ceiling, .5f);
    }
  }
  if (layout == 0 || layout == 2) {
    const loentity_ground_t* ceiling = loworld_template_add_ground_island_(
        param, &vec2(.7f, .38f), &vec2(.12f, .007f));
    if ((s = chaos_xorshift(s))%2) {
      loworld_template_add_random_enemy_(
          param, s = chaos_xorshift(s), ceiling, .5f);
    }
  }
}

void loworld_template_stairs_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  uint64_t s = param->seed;

  const loentity_ground_t* floor1 = loworld_template_add_ground_island_(
      param, &vec2(.5f, .3f), &vec2(.5f, .015f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), floor1, .5f);
  }

  bool layout = (s = chaos_xorshift(s))%2;
  const loentity_ground_t* floor2 = loworld_template_add_ground_island_(
      param, &vec2(layout? .3f: .6f, .5f), &vec2(.2f, .015f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), floor2, .5f);
  }

  layout = !layout;
  const loentity_ground_t* floor3 = loworld_template_add_ground_island_(
      param, &vec2(layout? .2f: .8f, .7f), &vec2(.18f, .007f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), floor3, .5f);
  }

  const loentity_ground_t* floor4 = loworld_template_add_ground_island_(
      param, &vec2(.5f, .9f), &vec2(.32f, .007f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_random_enemy_(
        param, s = chaos_xorshift(s), floor4, .5f);
  }
}

void loworld_template_boss_theists_child_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  loentity_ground_t* gnd = loworld_template_add_ground_island_(
      param, &vec2(.5f, .1f), &vec2(.5f, .05f));

  lochara_base_t* boss = lochara_pool_create(param->pools->chara);
  lochara_theists_child_build(boss, gnd);
  loworld_chunk_add_entity(param->target, &boss->super.super);
}

void loworld_template_boss_big_warder_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  loentity_ground_t* gnd = loworld_template_add_ground_island_(
      param, &vec2(.5f, .1f), &vec2(.5f, .05f));

  lochara_base_t* boss = lochara_pool_create(param->pools->chara);
  lochara_big_warder_build(boss, gnd);
  loworld_chunk_add_entity(param->target, &boss->super.super);
}

void loworld_template_boss_greedy_scientist_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  const loentity_ground_t* ground = loworld_template_add_ground_island_(
      param, &vec2(.5f, .1f), &vec2(.5f, .05f));
  (void) ground;
}
