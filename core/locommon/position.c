#include "./position.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

int32_t locommon_position_get_district_from_chunk(int32_t x) {
  static const int32_t cpd = LOCOMMON_POSITION_CHUNKS_PER_DISTRICT;
  return x >= 0? x/cpd: x/cpd-1;
}

int32_t locommon_position_get_chunk_index_in_district(int32_t x) {
  static const int32_t cpd = LOCOMMON_POSITION_CHUNKS_PER_DISTRICT;
  return x >= 0? x%cpd: cpd-1-(-x-1)%cpd;
}

bool locommon_position_valid(const locommon_position_t* a) {
  return a != NULL && vec2_valid(&a->fract);
}

void locommon_position_sub(
    vec2_t* a, const locommon_position_t* b, const locommon_position_t* c) {
  assert(a != NULL);
  assert(locommon_position_valid(b));
  assert(locommon_position_valid(c));

  vec2_sub(a, &b->fract, &c->fract);
  a->x += b->chunk.x - c->chunk.x;
  a->y += b->chunk.y - c->chunk.y;
}

void locommon_position_reduce(locommon_position_t* a) {
  assert(locommon_position_valid(a));

# define reduce_(e) do {  \
    a->chunk.e += (int) a->fract.e;  \
    a->fract.e -= (int) a->fract.e;  \
    if (a->fract.e < 0) {  \
      --a->chunk.e;  \
      ++a->fract.e;  \
    }  \
  } while (0)

  reduce_(x);
  reduce_(y);

# undef reduce_
}

void locommon_position_pack(
    const locommon_position_t* pos, msgpack_packer* packer) {
  assert(locommon_position_valid(pos));
  assert(packer != NULL);

  msgpack_pack_array(packer, 4);

  msgpack_pack_int32(packer, pos->chunk.x);
  msgpack_pack_int32(packer, pos->chunk.y);
  msgpack_pack_double(packer, pos->fract.x);
  msgpack_pack_double(packer, pos->fract.y);
}

bool locommon_position_unpack(
    locommon_position_t* pos, const msgpack_object* obj) {
  assert(pos != NULL);

  const msgpack_object_array* root = mpkutil_get_array(obj);
  if (root == NULL || root->size != 4 ||
      !mpkutil_get_int32(&root->ptr[0], &pos->chunk.x) ||
      !mpkutil_get_int32(&root->ptr[1], &pos->chunk.y) ||
      !mpkutil_get_float(&root->ptr[2], &pos->fract.x) ||
      !mpkutil_get_float(&root->ptr[3], &pos->fract.y)) {
    return false;
  }
  return true;
}
