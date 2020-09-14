#include "./counter.h"

#include <assert.h>
#include <stddef.h>

#include <msgpack.h>

#include "util/mpkutil/get.h"

void locommon_counter_initialize(locommon_counter_t* counter, size_t first) {
  assert(counter != NULL);

  *counter = (typeof(*counter)) {
    .next = first,
  };
}

void locommon_counter_deinitialize(locommon_counter_t* counter) {
  assert(counter != NULL);

}

size_t locommon_counter_count(locommon_counter_t* counter) {
  assert(counter != NULL);

  return counter->next++;
}

void locommon_counter_pack(
    const locommon_counter_t* counter, msgpack_packer* packer) {
  assert(counter != NULL);
  assert(packer  != NULL);

  msgpack_pack_uint64(packer, counter->next);
}

bool locommon_counter_unpack(
    locommon_counter_t* counter, const msgpack_object* obj) {
  assert(counter != NULL);

  if (!mpkutil_get_uint64(obj, &counter->next)) {
    return false;
  }
  return true;
}
