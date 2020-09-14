#include "./template.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/chaos/xorshift.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "core/locharacter/base.h"
#include "core/locharacter/big_warder.h"
#include "core/locharacter/cavia.h"
#include "core/locharacter/encephalon.h"
#include "core/locharacter/scientist.h"
#include "core/locharacter/greedy_scientist.h"
#include "core/locharacter/theists_child.h"
#include "core/locharacter/warder.h"
#include "core/locommon/position.h"
#include "core/loentity/entity.h"
#include "core/loground/base.h"
#include "core/loground/island.h"

#include "./chunk.h"
#include "./poolset.h"

static loentity_id_t loworld_template_add_ground_island_(
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
  return island->super.super.id;
}

static loentity_id_t loworld_template_add_character_random_enemy_(
    const loworld_template_building_param_t* param,
    uint64_t                                 seed,
    loentity_id_t                            ground,
    float                                    pos) {
  assert(param != NULL);

  locharacter_base_t* base = locharacter_pool_create(param->pools->character);

  bool built = false;
  const uint64_t type = (seed = chaos_xorshift(seed))%3;
  switch (type) {
  case 1:
    locharacter_warder_build(base, &(locharacter_warder_param_t) {
      .ground    = ground,
      .pos       = pos,
    });
    built = true;
    break;
  case 2:
    locharacter_scientist_build(base, &(locharacter_scientist_param_t) {
      .ground    = ground,
      .pos       = pos,
      .direction = (seed = chaos_xorshift(seed))%2? 1: -1,
    });
    built = true;
    break;
  }
  if (!built) {
    locharacter_cavia_build(base, &(locharacter_cavia_param_t) {
      .ground    = ground,
      .pos       = pos,
      .direction = (seed = chaos_xorshift(seed))%2? 1: -1,
    });
  }
  loworld_chunk_add_entity(param->target, &base->super.super);
  return base->super.super.id;
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

  loworld_template_add_ground_island_(
      param, &vec2(.5f, .2f), &vec2(.5f, .1f));

  const loentity_id_t ground = loworld_template_add_ground_island_(
      param, &vec2(.5f, .45f), &vec2(.2f, .02f));

  locharacter_base_t* encephalon =
      locharacter_pool_create(param->pools->character);
  locharacter_encephalon_build(encephalon, ground);
  loworld_chunk_add_entity(param->target, &encephalon->super.super);
}

void loworld_template_open_space_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  uint64_t s = param->seed;

  const size_t enemy_count = (s = chaos_xorshift(s))%3+1;

  const loentity_id_t ground = loworld_template_add_ground_island_(
      param, &vec2(.5f, .2f), &vec2(.47f, .01f));

  for (size_t i = 0; i < enemy_count; ++i) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), ground, (2.0f/enemy_count*i - 1)*.75f);
  }
}

void loworld_template_broken_open_space_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  uint64_t s = param->seed;

  const loentity_id_t floor1 = loworld_template_add_ground_island_(
      param, &vec2(.2f, .2f), &vec2(.18f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), floor1, .5f);
  }

  const loentity_id_t floor2 = loworld_template_add_ground_island_(
      param, &vec2(.8f, .2f), &vec2(.18f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), floor2, .5f);
  }

  loworld_template_add_ground_island_(
      param, &vec2(.5f, .05f), &vec2(.15f, .01f));
}

void loworld_template_passage_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  uint64_t s = param->seed;

  const loentity_id_t floor = loworld_template_add_ground_island_(
      param, &vec2(.5f, .25f), &vec2(.5f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), floor, .5f);
  }

  const loentity_id_t ceiling = loworld_template_add_ground_island_(
      param, &vec2(.55f, .4f), &vec2(.3f, .007f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), ceiling, .2f);
  }
}

void loworld_template_broken_passage_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  uint64_t s = param->seed;

  const loentity_id_t floor1 = loworld_template_add_ground_island_(
      param, &vec2(.15f, .25f), &vec2(.15f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), floor1, .5f);
  }

  const loentity_id_t floor2 = loworld_template_add_ground_island_(
      param, &vec2(.45f, .25f), &vec2(.1f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), floor2, .5f);
  }

  const loentity_id_t floor3 = loworld_template_add_ground_island_(
      param, &vec2(.85f, .25f), &vec2(.15f, .01f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), floor3, .5f);
  }

  const uint64_t layout = (s = chaos_xorshift(s))%3;
  if (layout == 0 || layout == 1) {
    const loentity_id_t ceiling = loworld_template_add_ground_island_(
        param, &vec2(.2f, .4f), &vec2(.15f, .007f));
    if ((s = chaos_xorshift(s))%2) {
      loworld_template_add_character_random_enemy_(
          param, (s = chaos_xorshift(s)), ceiling, .5f);
    }
  }
  if (layout == 0 || layout == 2) {
    const loentity_id_t ceiling = loworld_template_add_ground_island_(
        param, &vec2(.7f, .38f), &vec2(.12f, .007f));
    if ((s = chaos_xorshift(s))%2) {
      loworld_template_add_character_random_enemy_(
          param, (s = chaos_xorshift(s)), ceiling, .5f);
    }
  }
}

void loworld_template_stairs_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  uint64_t s = param->seed;

  const loentity_id_t floor1 = loworld_template_add_ground_island_(
      param, &vec2(.5f, .3f), &vec2(.5f, .015f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), floor1, .5f);
  }

  bool layout = (s = chaos_xorshift(s))%2;
  const loentity_id_t floor2 = loworld_template_add_ground_island_(
      param, &vec2(layout? .3f: .6f, .5f), &vec2(.2f, .015f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), floor2, .5f);
  }

  layout = !layout;
  const loentity_id_t floor3 = loworld_template_add_ground_island_(
      param, &vec2(layout? .2f: .8f, .7f), &vec2(.18f, .007f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), floor3, .5f);
  }

  const loentity_id_t floor4 = loworld_template_add_ground_island_(
      param, &vec2(.5f, .9f), &vec2(.32f, .007f));
  if ((s = chaos_xorshift(s))%2) {
    loworld_template_add_character_random_enemy_(
        param, (s = chaos_xorshift(s)), floor4, .5f);
  }
}

void loworld_template_boss_theists_child_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  const loentity_id_t ground = loworld_template_add_ground_island_(
      param, &vec2(.5f, .1f), &vec2(.5f, .05f));

  locharacter_base_t* boss = locharacter_pool_create(param->pools->character);
  locharacter_theists_child_build(boss, ground);
  loworld_chunk_add_entity(param->target, &boss->super.super);
}

void loworld_template_boss_big_warder_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  const loentity_id_t ground = loworld_template_add_ground_island_(
      param, &vec2(.5f, .1f), &vec2(.5f, .05f));

  locharacter_base_t* boss = locharacter_pool_create(param->pools->character);
  locharacter_big_warder_build(boss, ground);
  loworld_chunk_add_entity(param->target, &boss->super.super);
}

void loworld_template_boss_greedy_scientist_build_chunk(
    const loworld_template_building_param_t* param) {
  assert(loworld_template_building_param_valid(param));

  const loentity_id_t ground = loworld_template_add_ground_island_(
      param, &vec2(.5f, .1f), &vec2(.5f, .05f));

  locharacter_base_t* boss = locharacter_pool_create(param->pools->character);
  locharacter_greedy_scientist_build(boss, ground);
  loworld_chunk_add_entity(param->target, &boss->super.super);
}
