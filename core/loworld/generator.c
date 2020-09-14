#include "./generator.h"

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/chaos/xorshift.h"
#include "util/math/algorithm.h"
#include "util/memory/memory.h"
#include "util/mpkutil/get.h"

#include "./chunk.h"
#include "./poolset.h"
#include "./template.h"

struct loworld_generator_t {
  /* injected deps */
  const loworld_poolset_t* pools;

  /* parameters */
  uint64_t seed;
};

#define LOWORLD_GENERATOR_BLOCK_SIZE 8

static uint64_t loworld_generator_rand_(
    const loworld_generator_t* gen, int32_t x, int32_t y) {
  assert(gen != NULL);

  const uint64_t ux = (uint64_t) x - INT32_MIN;
  const uint64_t uy = (uint64_t) y - INT32_MIN;

  /* multiply prime numbers */
  return ux*400206850629133 + uy*890206850629189 + gen->seed;
}

static loworld_chunk_biome_t loworld_generator_decide_chunk_biome_(
    const loworld_generator_t* gen, int32_t x, int32_t y) {
  assert(gen != NULL);

  static const loworld_chunk_biome_t base_biomes[] = {
    LOWORLD_CHUNK_BIOME_CAVIAS_CAMP,
    LOWORLD_CHUNK_BIOME_LABORATORY,
  };
  static const size_t base_biomes_length =
      sizeof(base_biomes)/sizeof(base_biomes[0]);

  /* random seed which depends the block */
  uint64_t bs = loworld_generator_rand_(gen, x/LOWORLD_GENERATOR_BLOCK_SIZE, y);
  const loworld_chunk_biome_t base =
      base_biomes[(bs = chaos_xorshift(bs))%base_biomes_length];

  uint64_t fx = x + (y == 0? 0: loworld_generator_rand_(gen, 0, y));
  fx = MATH_ABS(fx)%LOWORLD_GENERATOR_BLOCK_SIZE;

  if (fx == 0) return LOWORLD_CHUNK_BIOME_METAPHYSICAL_GATE;
  if (fx == LOWORLD_GENERATOR_BLOCK_SIZE/2) {
    switch (base) {
    case LOWORLD_CHUNK_BIOME_CAVIAS_CAMP:
      return LOWORLD_CHUNK_BIOME_BOSS_THEISTS_CHILD;
    case LOWORLD_CHUNK_BIOME_LABORATORY:
      return x%2?
          LOWORLD_CHUNK_BIOME_BOSS_BIG_WARDER:
          LOWORLD_CHUNK_BIOME_BOSS_GREEDY_SCIENTIST;
    default: ;
    }
  }
  return base;
}

static void loworld_generator_generate_for_metaphysical_gate_(
    const loworld_generator_t* gen, loworld_chunk_t* chunk) {
  assert(gen   != NULL);
  assert(chunk != NULL);

  const loworld_template_building_param_t param = {
    .target = chunk,
    .pools  = gen->pools,
    .seed   = loworld_generator_rand_(gen, chunk->pos.x, chunk->pos.y),
  };
  loworld_template_metaphysical_gate_build_chunk(&param);
}

static void loworld_generator_generate_for_cavias_camp_(
    const loworld_generator_t* gen, loworld_chunk_t* chunk) {
  assert(gen   != NULL);
  assert(chunk != NULL);

  static void (*funcs[])(const loworld_template_building_param_t* p) = {
    loworld_template_open_space_build_chunk,
    loworld_template_broken_open_space_build_chunk,
    loworld_template_passage_build_chunk,
    loworld_template_broken_passage_build_chunk,
  };
  static const size_t funcs_len = sizeof(funcs)/sizeof(funcs[0]);

  uint64_t s = loworld_generator_rand_(gen, chunk->pos.x, chunk->pos.y);

  const uint64_t r = (s = chaos_xorshift(s))%funcs_len;

  const loworld_template_building_param_t param = {
    .target = chunk,
    .pools  = gen->pools,
    .seed   = chaos_xorshift(s),
  };
  return funcs[r](&param);
}

static void loworld_generator_generate_for_laboratory_(
    const loworld_generator_t* gen, loworld_chunk_t* chunk) {
  assert(gen   != NULL);
  assert(chunk != NULL);

  static void (*funcs[])(const loworld_template_building_param_t* p) = {
    loworld_template_passage_build_chunk,
    loworld_template_broken_passage_build_chunk,
    loworld_template_stairs_build_chunk,
  };
  static const size_t funcs_len = sizeof(funcs)/sizeof(funcs[0]);

  uint64_t s = loworld_generator_rand_(gen, chunk->pos.x, chunk->pos.y);

  const uint64_t r = (s = chaos_xorshift(s))%funcs_len;

  const loworld_template_building_param_t param = {
    .target = chunk,
    .pools  = gen->pools,
    .seed   = chaos_xorshift(s),
  };
  return funcs[r](&param);
}

static void loworld_generator_generate_for_boss_theists_child_(
    const loworld_generator_t* gen, loworld_chunk_t* chunk) {
  assert(gen   != NULL);
  assert(chunk != NULL);

  const loworld_template_building_param_t param = {
    .target = chunk,
    .pools  = gen->pools,
    .seed   = loworld_generator_rand_(gen, chunk->pos.x, chunk->pos.y),
  };
  loworld_template_boss_theists_child_build_chunk(&param);
}

static void loworld_generator_generate_for_boss_big_warder_(
    const loworld_generator_t* gen, loworld_chunk_t* chunk) {
  assert(gen   != NULL);
  assert(chunk != NULL);

  const loworld_template_building_param_t param = {
    .target = chunk,
    .pools  = gen->pools,
    .seed   = loworld_generator_rand_(gen, chunk->pos.x, chunk->pos.y),
  };
  loworld_template_boss_big_warder_build_chunk(&param);
}

static void loworld_generator_generate_for_boss_greedy_scientist_(
    const loworld_generator_t* gen, loworld_chunk_t* chunk) {
  assert(gen   != NULL);
  assert(chunk != NULL);

  const loworld_template_building_param_t param = {
    .target = chunk,
    .pools  = gen->pools,
    .seed   = loworld_generator_rand_(gen, chunk->pos.x, chunk->pos.y),
  };
  loworld_template_boss_greedy_scientist_build_chunk(&param);
}

loworld_generator_t* loworld_generator_new(
    const loworld_poolset_t* pools, uint64_t seed) {
  assert(pools != NULL);

  loworld_generator_t* gen = memory_new(sizeof(*gen));
  *gen = (typeof(*gen)) {
    .pools = pools,
  };
  loworld_generator_randomize(gen, seed);
  return gen;
}

void loworld_generator_delete(loworld_generator_t* gen) {
  assert(gen != NULL);

  memory_delete(gen);
}

void loworld_generator_randomize(loworld_generator_t* gen, uint64_t seed) {
  assert(gen != NULL);

  gen->seed = seed;
}

void loworld_generator_generate(
    const loworld_generator_t* gen, loworld_chunk_t* chunk) {
  assert(gen   != NULL);
  assert(chunk != NULL);

  chunk->biome =
      loworld_generator_decide_chunk_biome_(gen, chunk->pos.x, chunk->pos.y);

  switch (chunk->biome) {
  case LOWORLD_CHUNK_BIOME_METAPHYSICAL_GATE:
    loworld_generator_generate_for_metaphysical_gate_(gen, chunk);
    break;
  case LOWORLD_CHUNK_BIOME_CAVIAS_CAMP:
    loworld_generator_generate_for_cavias_camp_(gen, chunk);
    break;
  case LOWORLD_CHUNK_BIOME_LABORATORY:
    loworld_generator_generate_for_laboratory_(gen, chunk);
    break;
  case LOWORLD_CHUNK_BIOME_BOSS_THEISTS_CHILD:
    loworld_generator_generate_for_boss_theists_child_(gen, chunk);
    break;
  case LOWORLD_CHUNK_BIOME_BOSS_BIG_WARDER:
    loworld_generator_generate_for_boss_big_warder_(gen, chunk);
    break;
  case LOWORLD_CHUNK_BIOME_BOSS_GREEDY_SCIENTIST:
    loworld_generator_generate_for_boss_greedy_scientist_(gen, chunk);
    break;
  }
}

void loworld_generator_pack(
    const loworld_generator_t* gen, msgpack_packer* packer) {
  assert(gen    != NULL);
  assert(packer != NULL);

  msgpack_pack_uint64(packer, gen->seed);
}

bool loworld_generator_unpack(
    loworld_generator_t* gen, const msgpack_object* obj) {
  assert(gen != NULL);
  assert(obj != NULL);

  uint64_t seed;
  if (!mpkutil_get_uint64(obj, &seed)) return false;

  loworld_generator_randomize(gen, seed);
  return true;
}
