#include "./xorshift.h"

#include <stddef.h>
#include <stdint.h>

uint64_t chaos_xorshift(uint64_t seed) {
  seed = seed ^ (seed << 13);
  seed = seed ^ (seed >>  7);
  return seed ^ (seed << 17);
}

float chaos_xorshift_fract(uint64_t seed, uint64_t* next_seed) {
  static const uint64_t period = 10000;

  seed = chaos_xorshift(seed);
  if (next_seed != NULL) *next_seed = seed;

  return seed%period*1.f/period;
}
